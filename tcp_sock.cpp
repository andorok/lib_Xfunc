
#ifdef __linux__

#include <string.h>
#include <unistd.h>

#else

//#include <string.h>
//#include <stdlib.h>     /* atoi */

//#include <winsock.h>
#pragma comment(lib,"Ws2_32.lib")

#endif

#include "tcp_sock.h"

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
    m_sd = socket(AF_INET, SOCK_STREAM, 0 ); // AF_INET - address family

    // перевод сокета в неблокирующий режим
//    if(fcntl(m_sd, F_SETFL, O_NONBLOCK) == -1)
//        return -1;

    if( setsockopt(m_sd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) == -1)
        return -1;

#else
    m_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // перевод сокета в неблокирующий режим
//    int uMode = 1;
//    if (ioctlsocket(m_sd, FIONBIO, (u_long*)&uMode) == SOCKET_ERROR)
//        return -1;

    setsockopt(m_sd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)); // NODELAY
#endif

    return 0;
}

// Привязывание сокета к адресу (серверная функция)
int CTcpSock::bind(const std::string& ip, uint16_t port)
{
    struct sockaddr_in addr; // sockaddr_in - структура адреса сокета для интернет-протоколов
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET; // AF_INET - address family
    addr.sin_port = htons( port );

    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);
//#ifdef __linux__
//    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);
//    //addr.sin_addr.s_addr = ip->addr.ip;
//#else
//    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.S_un.S_addr);
//    //addr.sin_addr.S_un.S_addr = ip->addr.ip;
//#endif

    // sockaddr - структура для адреса сокета
    if(::bind( m_sd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        return SOCKET_ERROR;

     return 0;
}

// Прослушивание порта в ожидании входящих соединений (серверная функция)
int CTcpSock::listen(int max_clients)
{
    return ::listen(m_sd, max_clients);
}

// Принятие входящего соединения (с созданием для него нового сокета)
// Если ожимдающий соединений нет, то происходит блокировка внутри функции ::accept
// Возвращает дескриптор нового (клиентского) сокета
#ifdef __linux__
int CTcpSock::accept(const std::string& ip_str, uint16_t& port)
#else
SOCKET CTcpSock::accept(const std::string& ip_str, uint16_t& port)
#endif
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    int len = sizeof(addr);

#ifdef __linux__
    int client_sd;
    client_sd = ::accept(m_sd, (sockaddr*)&addr, (socklen_t*)&len);
#else
    SOCKET client_sd;
    client_sd = ::accept(m_sd, (sockaddr*)&addr, &len);
#endif

    if (client_sd == -1)
        return client_sd;

    if (port)
    {
        port = ntohs(addr.sin_port);
        inet_ntop(AF_INET, &(addr.sin_addr), (char*)ip_str.c_str(), INET_ADDRSTRLEN);
        //printf("%s\n", ip_str.c_str());
    }

    return client_sd;
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

    printf("connect: err %d\n", err);

    return err;
}

int CTcpSock::send(char* data, int size, int flags)
{
    return ::send(m_sd, data, size, flags);
}

int CTcpSock::recv( char *data, int size, int flags)
{
    return ::recv( m_sd, data, size, flags);
}

int32_t CTcpSock::send_all(char* data, int len, int flags)
{
    int total = 0;
    int cnt = 0;
    int size = len;
    while (size > 0)
    {
        cnt = ::send(m_sd, data, size, flags);
        if (cnt == -1)
            return SOCKET_ERROR;
        data += cnt;
        size -= cnt;
        total += cnt;
    }

    return total;
}

int32_t CTcpSock::recv_all(char* data, int len, int flags)
{
    int total = 0;
    int cnt = 0;
    int size = len;
    while (size > 0)
    {
        cnt = ::recv(m_sd, data, size, flags);
        //printf("    CTcpSock::%s: data portion %d of %d\n", __func__, cnt, size);
        if (cnt == -1)
        {
            printf("    CTcpSock::%s: %s (%d)\n", __func__, strerror(errno), errno);
            return SOCKET_ERROR;
        }
        if (cnt == 0)
        {
            printf("\n  CTcpSock::%s: disconnected (received data size 0)\n", __func__);
            return 0; // разрыв соединения
        }
        total += cnt;
        printf("    CTcpSock::%s: data portion %d of %d (total %d of %d)\n", __func__, cnt, size, total, len);
        data += cnt;
        size -= cnt;
    }

    return total;
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
