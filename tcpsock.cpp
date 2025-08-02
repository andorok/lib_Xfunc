
#ifdef __linux__

#include <string.h>
#include <unistd.h>

#else

//#include <string.h>
//#include <stdlib.h>     /* atoi */

//#include <winsock.h>
#pragma comment(lib,"Ws2_32.lib")

#endif

#include "tcpsock.h"

CTcpSock::CTcpSock( )
{
    m_sd = -1;
#ifdef _WIN32

    // Initialize Winsock
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    int ret = WSAStartup(wVersionRequested, &wsaData);
    if (ret != NO_ERROR) {
        printf("Error at WSAStartup()\n");

    }
#endif
}

CTcpSock::~CTcpSock()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

int CTcpSock::open()
{
    int flag = 1;
#ifdef __linux__
    m_sd = socket(AF_INET, SOCK_STREAM, 0 );

//    if(fcntl(m_sd, F_SETFL, O_NONBLOCK) == -1)
//        return -1;

    if( setsockopt(m_sd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) == -1)
        return -1;

#else
    m_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int uMode = 1;
    if (ioctlsocket(m_sd, FIONBIO, (u_long*)&uMode) == SOCKET_ERROR)
        return -1;

    setsockopt(m_sd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)); // NODELAY
#endif

    return 0;
}

int CTcpSock::listen( int backlog )
{
//#ifdef __linux__
    return ::listen(m_sd, backlog);
//#else
//    return ::listen(backlog);
//#endif
}

int CTcpSock::bind(const std::string& ip, uint16_t port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( port );

#ifdef __linux__
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);
    //addr.sin_addr.s_addr = ip->addr.ip;
#else
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.S_un.S_addr);
    //addr.sin_addr.S_un.S_addr = ip->addr.ip;
#endif

    if(::bind( m_sd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        return SOCKET_ERROR;

     return 0;
}

int CTcpSock::connect(const std::string& ip, uint16_t port)
{
    sockaddr_in addr;
    memset( &addr, 0, sizeof(addr) );

    addr.sin_family = AF_INET;                   // Инициализация сокета и параметров сети
#ifdef __linux__
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);
    //addr.sin_addr.s_addr = ip->addr.ip;
#else
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.S_un.S_addr);
    //addr.sin_addr.S_un.S_addr = ip->addr.ip;
#endif
    addr.sin_port = htons(port);

//    printf("connect to %s:%d\n", ip.c_str(), port);

    int err = :: connect(m_sd, (struct sockaddr*)&addr, sizeof(struct sockaddr));

//    printf("connect: err %d\n", err);

    return err;
}

int CTcpSock::send( char *data, int size, int timeout )
{
    return ::send( m_sd, data, size, 0 );
}

int CTcpSock::recv( char *data, int size, int timeout  )
{
    return ::recv( m_sd, data, size, 0 );
}

int CTcpSock::close()
{
    if(!m_sd)
        return -1;

#ifdef _WIN32
    int uMode = 0;
    if (ioctlsocket(m_sd, FIONBIO, (u_long*)&uMode) == SOCKET_ERROR)
        return SOCKET_ERROR;
    if ( closesocket(m_sd) == -1)
#else
    if( ::close(m_sd) == -1)
#endif
        return -1;

    return 0;
}
