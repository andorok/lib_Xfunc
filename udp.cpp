#include "udp.h"

#include <cstring>

#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>

Udp::Udp():
    sd( -1 )
{

}

Udp::~Udp()
{
    if( sd > 0 )
    {
        if( mcast )
            setsockopt( sd, SOL_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq) );

        ::close( sd );
        sd = -1;
    }
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

    if( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0 )
    {
        ::close( sd );
        sd = 0;
        return -1;
    }

    struct sockaddr_in  socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;		 // AF_INET - address family
    socket_address.sin_port = htons( src_port );
    socket_address.sin_addr.s_addr = INADDR_ANY;
    if( bind(sd, (struct sockaddr *) &socket_address, sizeof(socket_address)) != 0 )
    {
        ::close( sd );
        sd = 0;
        return -1;
    }

    memset( &recipient, 0, sizeof(recipient) );
    inet_pton( AF_INET, ip.c_str(), &recipient.sin_addr.s_addr );
    recipient.sin_port = htons( dst_port );

    return 0;
}

ssize_t Udp::sendto(void *buf, size_t n, int flags)
{
    return ::sendto( sd, buf, n, flags, (const sockaddr*)&recipient, sizeof(recipient) );
}

ssize_t Udp::sendtoaddress(const std::string &ip, uint16_t dst_port, void *buf, size_t n, int flags)
{
    struct sockaddr_in recipient;
    memset( &recipient, 0, sizeof(recipient) );

    inet_pton( AF_INET, ip.c_str(), &recipient.sin_addr.s_addr );
    recipient.sin_port = htons( dst_port );

    return ::sendto( sd, buf, n, flags, (const sockaddr*)&recipient, sizeof(recipient) );
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

    if( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0 )
    {
        ::close( sd );
        sd = 0;
        return -1;
    }

    memset( &multiAddr, 0, sizeof(multiAddr) );
    inet_aton( ip.c_str(), &multiAddr );
    if( ( multiAddr.s_addr & 0xE0 ) == 0xE0)
    {
        mcast = true;
        memset( &mreq, 0, sizeof(mreq) );
        mreq.imr_multiaddr = multiAddr;
        mreq.imr_interface.s_addr = INADDR_ANY;
        if( setsockopt( sd, SOL_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq) ) != 0 )
        {
            ::close( sd );
            sd = 0;
            return -1;
        }
    }
    else
        mcast = false;

    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons( localPort );
    socket_address.sin_addr = local_address;
    if( bind(sd, (struct sockaddr *) &socket_address, sizeof(socket_address)) != 0 )
    {
        ::close( sd );
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
    socklen_t len = sizeof(sockaddr_in);
    return ::recvfrom( sd, buf, n, flags, (struct sockaddr *) addr, &len );
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
        if( mcast )
            setsockopt( sd, SOL_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq) );

        res = ::close( sd );
        sd = -1;
    }

    return res;
}

