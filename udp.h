#ifndef UDP_H
#define UDP_H

#include <stdint.h>

#include <cstdio>
#include <cstring>

#include <string>

#ifdef __linux__
#include <arpa/inet.h>
#else
#include <windows.h>
#define ssize_t ptrdiff_t
#endif

class Udp
{
public:
    Udp();
    ~Udp();

    /**
     * @brief init_send Подготовить сокет для отправки пакетов
     * @param ip        - адрес получателя
     * @param port      - порт получателя
     * @return
     */
    int init_send(const std::string &ip, uint16_t dst_port, uint16_t src_port = 0);
    ssize_t sendto(void *buf, size_t n, int flags);

    /**
     * @brief init Подготовить сокет для приема пакетов
     * @param ip        - ip-адрес интерфейса, на котором ожидаются пакеты, в т.ч. IPADDR_ANY и multicast
     * @param port      - порт, на котором ожидаются пакеты
     * @return
     */
    int init(const std::string &ip, uint16_t port);

    int get_sd();
    int close();

    int sd;
    bool mcast;
    struct ip_mreq mreq;

    struct sockaddr_in recipient;

    ssize_t sendtoaddress(const std::string &ip, uint16_t dst_port, void *buf, size_t n, int flags);

    ssize_t recvfrom(void *buf, size_t n, int flags = 0, sockaddr_in *addr = nullptr);
};

#endif // UDP_H
