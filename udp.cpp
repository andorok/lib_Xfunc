#include "udp.h"

#include <cstring>

#ifdef __linux__
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#else
// код под windows только компилировался (не отлаживался и не запускался)
// некоторый код под windows просто закомментирован
//#include <winsock2.h>
#pragma comment(lib,"Ws2_32.lib")
//#include <winsock2.h>
//#include <ws2tcpip.h>
#endif

Udp::Udp():
    sd( -1 )
{
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != NO_ERROR) {
        printf("Error at WSAStartup()\n");
    }
#endif
}

Udp::~Udp()
{
#ifdef __linux__
    if (sd > 0)
    {
        if( mcast )
            setsockopt( sd, SOL_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq) );
        ::close(sd);
        sd = -1;
    }
#else
    if (sd > 0)
    {
        if (mcast)
            setsockopt(sd, SOL_IP, IP_DROP_MEMBERSHIP, (const char*)&mreq, sizeof(mreq));

        closesocket(sd);
        sd = -1;
    }
    WSACleanup();
#endif
}

int Udp::init_send(const std::string &ip, uint16_t dst_port, uint16_t src_port)
{
    int one = 1;
    sd = socket ( PF_INET, SOCK_DGRAM, 0 ); // PF_INET - protocol family
    if( sd == -1 )
    {
        sd = 0;
        return -1;
    }

#ifdef __linux__
    if( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0 )
    {
        ::close(sd);
        sd = 0;
        return -1;
    }
#else
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one)) != 0)
    {
        closesocket(sd);
        sd = 0;
        return -1;
    }
#endif

    struct sockaddr_in  socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;		 // AF_INET - address family
    socket_address.sin_port = htons( src_port );
    socket_address.sin_addr.s_addr = INADDR_ANY;
    if( bind(sd, (struct sockaddr *) &socket_address, sizeof(socket_address)) != 0 )
    {
#ifdef __linux__
        ::close(sd);
#else
        closesocket(sd);
#endif
        sd = 0;
        return -1;
    }

    memset( &recipient, 0, sizeof(recipient) );

    inet_pton(AF_INET, ip.c_str(), &recipient.sin_addr.s_addr);
    recipient.sin_port = htons( dst_port );

    return 0;
}

ssize_t Udp::sendto(void *buf, size_t n, int flags)
{
    return ::sendto( sd, (char*)buf, n, flags, (const sockaddr*)&recipient, sizeof(recipient) );
}

ssize_t Udp::sendtoaddress(const std::string &ip, uint16_t dst_port, void *buf, size_t n, int flags)
{
    struct sockaddr_in recipient;
    memset( &recipient, 0, sizeof(recipient) );

    inet_pton( AF_INET, ip.c_str(), &recipient.sin_addr.s_addr );
    recipient.sin_port = htons( dst_port );

    return ::sendto( sd, (char*)buf, n, flags, (const sockaddr*)&recipient, sizeof(recipient) );
}

int Udp::init(const std::string &ip, uint16_t port)
{
    int one = 1;
    struct in_addr      multiAddr;
    struct in_addr      local_address;
    struct sockaddr_in  socket_address;

    local_address.s_addr = INADDR_ANY;

    uint16_t localPort = port;

    sd = socket ( PF_INET, SOCK_DGRAM, 0 );
    if( sd == -1 )
    {
        sd = 0;
        return -1;
    }

#ifdef __linux__
    if( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0)
    {
        ::close(sd);
        sd = 0;
        return -1;
    }
#else
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one)) != 0)
    {
        closesocket(sd);
        sd = 0;
        return -1;
    }
#endif

    memset( &multiAddr, 0, sizeof(multiAddr) );
#ifdef __linux__
    inet_aton( ip.c_str(), &multiAddr );
#else
    inet_pton(AF_INET, ip.c_str(), &multiAddr);
#endif
    if( ( multiAddr.s_addr & 0xE0 ) == 0xE0)
    {
        mcast = true;
        memset( &mreq, 0, sizeof(mreq) );
        mreq.imr_multiaddr = multiAddr;
        mreq.imr_interface.s_addr = INADDR_ANY;
#ifdef __linux__
        if( setsockopt( sd, SOL_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq) ) != 0 )
        {
            ::close(sd);
            sd = 0;
            return -1;
        }
#else
        if (setsockopt(sd, SOL_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq)) != 0)
        {
            closesocket(sd);
            sd = 0;
            return -1;
        }
#endif
    }
    else
        mcast = false;

    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons( localPort );
    socket_address.sin_addr = local_address;
    if( bind(sd, (struct sockaddr *) &socket_address, sizeof(socket_address)) != 0 )
    {
#ifdef __linux__
        ::close(sd);
#else
        closesocket(sd);
#endif
        sd = 0;
        return -1;
    }

//    struct timeval t;
//    t.tv_sec = 0;
//    t.tv_usec = 100;

//    if( setsockopt( sd, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t) ) != 0 )
//    {
//        close( sd );
//        sd = 0;
//        return -1;
//    }

    return 0;
}

ssize_t Udp::recvfrom(void *buf, size_t n, int flags, struct sockaddr_in *addr)
{
#ifdef __linux__
    socklen_t len = sizeof(sockaddr_in);
#else
    int len = sizeof(sockaddr_in);
#endif
    return ::recvfrom( sd, (char*)buf, n, flags, (struct sockaddr *) addr, &len );
}

int Udp::get_sd()
{
    return sd;
}

int Udp::close()
{
    int res = -1;
    if( sd > 0 )
    {
        if (mcast)
#ifdef __linux__
            setsockopt(sd, SOL_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
        res = ::close(sd);
#else
            setsockopt(sd, SOL_IP, IP_DROP_MEMBERSHIP, (const char*)&mreq, sizeof(mreq));
        res = closesocket(sd);
#endif
        sd = -1;
    }

    return res;
}

