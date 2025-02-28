//=************************* CTcpSock  - the TCP socket functions class *************************

#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

#ifdef __linux__

#include <sys/select.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCKET_ERROR (-1)

#else

#include <winsock2.h>
#include <ws2tcpip.h>
//#include <Winsock.h>

#endif // __linux__

#include <string>

class CTcpSock
{

#ifdef __linux__
    int m_sd;
#else
    SOCKET m_sd;
#endif

public:
    CTcpSock();
    ~CTcpSock();

    int	open();
    int connect(const std::string& ip, uint16_t port);
    int	close();
    int send(char* data, int size, int timeout);
    int recv(char* data, int size, int timeout);

    int listen(int backlog);
    int bind(const std::string& ip, uint16_t port);
};

#endif // _TCPSOCKET_H_
