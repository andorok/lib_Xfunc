//=************************* CTcpSock  - the TCP socket functions class *************************

#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

#ifdef __linux__

#include <sys/select.h>
#include <fcntl.h>
#include <netdb.h>

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define INVALID_SOCKET  (~0)
#define SOCKET_ERROR (-1)

//#define MAX_CLIENTS     SOMAXCONN

#else

#include <winsock2.h>
#include <ws2tcpip.h>
//#include <Winsock.h>

#endif // __linux__

#include <string>

//#define MAX_CLIENTS     4

class CTcpSock
{
protected:

#ifdef __linux__
    int m_sd;
#else
    SOCKET m_sd;
#endif

public:
    CTcpSock();
    ~CTcpSock();

    int	open();
    int	close();
    int send(char* data, int size, int flags = 0);
    int recv(char* data, int size, int flags = 0);
    int32_t send_all(char* data, int len, int flags = 0);
    int32_t recv_all(char* data, int len, int flags = 0);

    // server function
    int listen(int max_clients);
    int bind(const std::string& ip, uint16_t port);
#ifdef __linux__
    int accept(const std::string& ip_str, uint16_t& port);
#else
    SOCKET accept(const std::string& ip_str, uint16_t& port);
#endif

    // client function
    int connect(const std::string& ip, uint16_t port);
};

#endif // _TCPSOCKET_H_
