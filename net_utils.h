#pragma once

#include <stdint.h>

#ifdef __linux__
#include<stdio.h>   //printf
#include<string.h>  //memset
#include<errno.h>   //errno
#include<sys/socket.h>  //socket
#include<netinet/in.h> //sockaddr_in
#include<arpa/inet.h>   //getsockname
#include<unistd.h>  //close
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#else 
//#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

#ifdef __linux__
uint32_t getCurentIP()
{
    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    snprintf(ifr.ifr_name, IFNAMSIZ, "eth0");
    ioctl(fd, SIOCGIFADDR, &ifr);
    /* and more importantly */
    //printf("%s\n", inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));
    uint32_t HostIP = (uint32_t)(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr);
    close(fd);
    return HostIP;
}


//int main(int argc, char* argv[])
//{
//    const char* google_dns_server = "8.8.8.8";
//    int dns_port = 53;
//    struct sockaddr_in serv;
//    int sock = socket(AF_INET, SOCK_DGRAM, 0);
//    //Socket could not be created
//    if (sock < 0)    {
//        perror("Socket error");
//    }
//
//    memset(&serv, 0, sizeof(serv));
//    serv.sin_family = AF_INET;
//    serv.sin_addr.s_addr = inet_addr(google_dns_server);
//    serv.sin_port = htons(dns_port);
//
//    int err = connect(sock, (const struct sockaddr*)&serv, sizeof(serv));
//
//    struct sockaddr_in name;
//    socklen_t namelen = sizeof(name);
//    err = getsockname(sock, (struct sockaddr*)&name, &namelen);
//
//    char buffer[100];
//    const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, 100);
//
//    if (p != NULL)
//    {
//        printf("Local ip is : %s \n", buffer);
//    }
//    else
//    {
//        //Some error
//        printf("Error number : %d . Error message : %s \n", errno, strerror(errno));
//    }
//
//    close(sock);
//
//    return 0;
//}

#else 
uint32_t getCurentIP()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(1, 1), &wsaData); // инициализируем socket'ы используя Ws2_32.dll для процесса

    char HostName[1024]; // создаем буфер для имени хоста
    uint32_t HostIP = 0;

    if (!gethostname(HostName, 1024)) // получаем имя хоста
    {
        if (LPHOSTENT lphost = gethostbyname(HostName)) // получаем IP хоста, т.е. нашего компа
        {
            HostIP = ((LPIN_ADDR)lphost->h_addr)->s_addr; // преобразуем переменную типа LPIN_ADDR в DWORD
            //printf("%s\n", inet_ntoa(lphost->h_addr );
        }
    }
    WSACleanup(); // освобождаем сокеты, т.е. завершаем использование Ws2_32.dll
    return HostIP;
}
#endif
