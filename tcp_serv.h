//=************************* CTcpServ  - the TCP server functions class *************************

#ifndef _TCPSERV_H_
#define _TCPSERV_H_

#ifdef __linux__

#include <sys/select.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>

//#define SOCKET_ERROR (-1)

//#define MAX_CLIENTS     SOMAXCONN

#else

//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <Winsock.h>

#endif // __linux__

//#include <string>

#include    "tcp_sock.h"
#include    "cthread.h"

#define MAX_CLIENTS     4
#define DEFAULT_PORT    7788

class CTcpServ;

typedef struct CLIENT_PARAMS {
    int client_idx;          // client index
    CTcpServ* tcp_server;    // pointer to TCP server
    void* user_data;         // pointer to user data
} CLIENT_PARAMS;

class CTcpServ : public CTcpSock
{
    uint32_t m_port;        // порт сервера
#ifdef __linux__
    int      m_client_sd[MAX_CLIENTS];   // clients sockets
    void* (*m_client_service)(void* pParams); // указатель на внешнюю функцию, вызываемую из потоки обслуживани€€ клиента
#else
    SOCKET m_client_sd[MAX_CLIENTS];   // clients sockets
    // указатель на внешнюю функцию, вызываемую из потоки обслуживани€€ клиента
    // unsigned int __stdcall (*m_client_service)(void* pParams); 
    unsigned int (*m_client_service)(void* pParams);
#endif
    struct sockaddr_in m_srv_addr;
    int     m_clients;                    // current number clients

    CLIENT_PARAMS m_params[MAX_CLIENTS];
    CThread m_client_threads[MAX_CLIENTS]; // потоки обслуживани€€ клиента
    int m_threads_status[MAX_CLIENTS]; // состо€ние потоков обслуживани€€ клиента
    void* m_user_data;         // pointer to user data

public:
    CTcpServ(uint32_t port);
    CTcpServ();
    ~CTcpServ();

    int32_t	open();
    int32_t close();
    int32_t set_port(uint32_t port);
#ifdef __linux__
    int32_t set_client_service(void* (*func) (void*), void* user_data);
#else
    int32_t set_client_service(unsigned int (*func) (void*), void* user_data);
#endif

    int32_t connect(int client_idx);
    int32_t connect2(bool* stop_flag, int dbg_flg = 0);
    int32_t disconnect(int client_idx);

    int32_t recv(int client_idx, void* buf, uint32_t size);
    int32_t send(int client_idx, void* buf, uint32_t size);

    int32_t recv_all(int client_idx, void* buf, int len);
    int32_t send_all(int client_idx, void* buf, int len);
};

#endif // _TCPSERV_H_
