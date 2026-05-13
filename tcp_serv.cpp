
#include "tcp_serv.h"

// получаем адрес сокета, ipv4 или ipv6:
static void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


CTcpServ::~CTcpServ()
{
}

// General constructor
CTcpServ::CTcpServ(uint32_t port) :
    CTcpSock()
{
    m_port = port;
    
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        m_client_sd[i] = INVALID_SOCKET;
        m_params[i].client_idx = -1;
        m_params[i].tcp_server = nullptr;
    }

}

// default constructor
CTcpServ::CTcpServ() :
    CTcpSock()
{
    m_port = DEFAULT_PORT;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        m_client_sd[i] = INVALID_SOCKET;
        m_params[i].client_idx = -1;
        m_params[i].tcp_server = nullptr;
    }
}

int32_t CTcpServ::set_port(uint32_t port)
{
    m_port = port;
    return m_port;
}

#ifdef __linux__
int32_t CTcpServ::set_client_service(void* (*func) (void*), void* user_data)
#else
int32_t CTcpServ::set_client_service(unsigned int (*func) (void*), void* user_data)
#endif
{
    m_client_service = func;
    m_user_data = user_data;
    printf("CTcpServ:: set Client Function\n");
    return 0;
}

int32_t CTcpServ::open()
{
#ifdef __linux__
    m_sd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET - address family
#else
    m_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // AF_INET - address family
#endif
    if (m_sd < 0){
        printf("CTcpServ:: [-]Socket error");
        return -1;
    }
    printf("CTcpServ:: [+]TCP server socket created %d.\n", m_sd);

    // SO_REUSEADDR - позволяет сокету принудительно привязаться к порту, используемому другим сокетом
    int one = 1; 
#ifdef __linux__
    setsockopt(m_sd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#else
    setsockopt(m_sd, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one));
#endif

    struct sockaddr_in srv_addr; // sockaddr_in - структура адреса сокета для интернет-протоколов
    memset(&srv_addr, 0, sizeof(sockaddr_in));
    srv_addr.sin_family = AF_INET; // AF_INET - address family
    srv_addr.sin_port = htons(m_port);
    srv_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr(ip);

    if(::bind(m_sd, (struct sockaddr*)&srv_addr, sizeof(sockaddr_in)) == SOCKET_ERROR){
        printf("CTcpServ:: [-] Bind error: \n");
        printf(" %s (%d)\n", strerror(errno), errno);
        return SOCKET_ERROR;
    }
    printf("CTcpServ:: [+]Bind to the port number: %d\n", m_port);

    if(::listen(m_sd, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("CTcpServ:: [-]Listen error");
        return SOCKET_ERROR;
    }
    printf("CTcpServ:: Listening...\n");

    // zeroed client services
    //for(int i = 0; i < MAX_CLIENTS; i++)
    //    m_client_threads[i] = NULL;
    m_clients = 0;  // текущее число активных клиентов

    return 1;
}

int32_t CTcpServ::close()
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if ((m_client_sd[i] == INVALID_SOCKET) && (m_params[i].client_idx == i))
        {
            m_client_threads[i].wait(INFINITE); // Wait until threads terminates
            m_client_threads[i].close();
            m_params[i].client_idx = -1;
            m_params[i].tcp_server = nullptr;
            printf("\n CTcpServ:: client thread %d closed\n", i);
        }
    }
    CTcpSock::close();
    //close(m_server_sock);
    return 0;
}

int32_t CTcpServ::connect(int client_idx)
{
    // ожидаем соединения с клиентом -----------------------------------
    sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    //int len = sizeof(client_addr);
    socklen_t addr_size = sizeof(sockaddr);
    m_client_sd[client_idx] = ::accept(m_sd, (struct sockaddr*)&client_addr, &addr_size);

    //int port = client_addr[client_idx].sin_port;
    printf("CTcpServ:: [+] Client Port = %d\n", client_addr.sin_port);

    printf("CTcpServ:: [+]Client connected.\n");
    // устанавливаем посылку без ожидания доставки (для скорости)
    int flag = 1;
    setsockopt(m_client_sd[client_idx], IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
    char s[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET, get_in_addr((struct sockaddr *)&client_addr), s, sizeof s);

    // перевод сокета в неблокирующий режим
#ifdef __linux__
    fcntl(m_client_sd[client_idx], F_SETFL, O_NONBLOCK);
#else
    int uMode = 1;
    ioctlsocket(m_client_sd[client_idx], FIONBIO, (u_long*)&uMode);
#endif

    struct timeval tv;
    tv.tv_sec = 10;//timeout_in_seconds;
    tv.tv_usec = 0;
    setsockopt(m_client_sd[client_idx], SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    printf("+++++ CTcpServ:: server: got connection from %s\n", s);
    // Есть соединение с клиентом -----------------------------------

    // start new client service
    CLIENT_PARAMS params;
    params.client_idx = client_idx;   // current client number
    params.tcp_server = this;       // tcp server pointer
    m_client_threads[client_idx].create(m_client_service, &params);

    m_clients++;
    printf("+++++ CTcpServ:: Add client = %d\n", m_clients);

    return 0;
}

int32_t CTcpServ::connect2(bool* stop_flag, int dbg_flg)
{
    if (m_clients >= MAX_CLIENTS)
        return 2;

    int retval = 0;
    int nsel = 0;
    fd_set  fds;
    while (*stop_flag == 0)
    {
        // Maximum wait time for the "select" command
        timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 1;
        // Initialize the fd_set structure to NULL
        FD_ZERO(&fds);
        // Add the sckServer socket to fd_set structure
        FD_SET(m_sd, &fds);
        // Call the select command
        //if (select(m_sd+1, &fds, NULL, NULL, &tv) != 0)
        retval = select(m_sd + 1, &fds, NULL, NULL, &tv);
        if (retval != 0)
            // Maximum wait time is expired.
            break;
        if(dbg_flg)
            printf(" CTcpServ:: select timeout %d sec.\r", nsel++);
    }
    if (retval == 0)
        return -1;

    if (retval == -1)
    {
        //perror("select()");
        printf("CTcpServ:: [-] Select error: \n");
        printf(" %s (%d)\n", strerror(errno), errno);
        return -1;
    }
    //else
    //        printf("Data is available now.\n");
    ///* FD_ISSET(0, &rfds) will be true. */
    //    else
    //        printf("No data within five seconds.\n");

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if ((m_client_sd[i] == INVALID_SOCKET) && (m_params[i].client_idx == i))
        {
            m_client_threads[i].wait(INFINITE); // Wait until threads terminates
            m_client_threads[i].close();
            m_params[i].client_idx = -1;
            m_params[i].tcp_server = nullptr;
            printf("\n CTcpServ:: client thread %d closed\n", i);
        }
    }

    int client_idx = m_clients;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (m_client_sd[i] == INVALID_SOCKET) {
            client_idx = i;
            break;
        }
    }
    printf("\n CTcpServ:: client index %d\n", client_idx);

    //printf("\n");
    // Check is there any incoming request/active in the fd_set structure
    // Zero mean no, else
    if (FD_ISSET(m_sd, &fds) != 0)
    {
        printf("CTcpServ:: Accepting...\n");
        sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        //int len = sizeof(client_addr);
        socklen_t addr_size = sizeof(sockaddr);
        m_client_sd[client_idx] = ::accept(m_sd, (struct sockaddr*)&client_addr, &addr_size);

        //int port = client_addr[client_idx].sin_port;
        printf("CTcpServ:: [+] Client Port = %d\n", client_addr.sin_port);

        printf("CTcpServ:: [+]Client connected.\n");
        // устанавливаем посылку без ожидания доставки (для скорости)
        int flag = 1;
        setsockopt(m_client_sd[client_idx], IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
        char s[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET, get_in_addr((struct sockaddr*)&client_addr), s, sizeof s);

        // перевод сокета в неблокирующий режим
#ifdef __linux__
        fcntl(m_client_sd[client_idx], F_SETFL, O_NONBLOCK);
#else
        int uMode = 1;
        ioctlsocket(m_client_sd[client_idx], FIONBIO, (u_long*)&uMode);
#endif

        struct timeval tv;
        tv.tv_sec = 10;//timeout_in_seconds;
        tv.tv_usec = 0;
        setsockopt(m_client_sd[client_idx], SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        printf("CTcpServ:: server: got connection from %s:%d\n", s, m_port);
        // Есть соединение с клиентом -----------------------------------

        // start new client service
        //CLIENT_PARAMS params;
        m_params[client_idx].client_idx = client_idx;   // current client number
        m_params[client_idx].tcp_server = this;       // tcp server pointer
        m_params[client_idx].user_data = m_user_data; // user data pointer
        m_client_threads[client_idx].create(m_client_service, &m_params[client_idx]);

        m_clients++;
        printf("CTcpServ:: Add client %d -> Total clients %d\n", client_idx, m_clients);
    }
    return 0;
}

int32_t CTcpServ::disconnect(int client_idx)
{
#ifdef __linux__
    if (::close(m_client_sd[client_idx]) == -1)
#else
    if (closesocket(m_client_sd[client_idx]) == -1)
#endif
        return -1;

    printf("CTcpServ::disconnect: Close Client %d\n", client_idx);

    m_client_sd[client_idx] = INVALID_SOCKET;

    m_clients--;
    printf("+++++ CTcpServ:: Minus client %d -> Total clients %d\n", client_idx, m_clients);

    //m_hThread_service[client_idx] = 0;
    //printf("CTcpServ:: Close m_hThread_service[%d] = %d\n", client_idx, (pthread_t)m_hThread_service[client_idx]);
    return 0;
}

int32_t CTcpServ::recv(int client_idx, void *buf, uint32_t size)
{
    int32_t    _size;
    do {
        _size = ::recv(m_client_sd[client_idx], (char*)buf, size, 0);
    } while(_size==-1);

    return _size;
}

int32_t CTcpServ::send(int client_idx, void *buf, uint32_t size)
{
    int32_t    _size;
    do {
        _size = ::send(m_client_sd[client_idx], (char*)buf, size, 0);
    } while(_size==0);

    return _size;
}

//int32_t CTcpServ::send_all(int client_idx, void* buf, int len)
//{
//    char* data = (char*)buf;
//    int total = 0;
//    int portion_size = 1440;
//    int cnt = len / portion_size;
//    int ost = len % portion_size;
//    int size = len;
//
//    for (int i = 0; i < cnt; i++)
//    {
//        int _size = ::send(m_client_sd[client_idx], data, portion_size, 0);
//        printf("    CTcpServ::send_all: data portion %d of %d [%d of %d bytes]\n", i, cnt, _size, portion_size);
//        data += portion_size;
//        total += _size;
//        printf("    CTcpServ::send_all: data total %d of %d \n", total, len);
//    }
//    int _size = ::send(m_client_sd[client_idx], data, ost, 0);
//    total += _size;
//    printf("    CTcpServ::send_all: data portion %d of %d [%d of %d bytes]\n", _size, ost, total, len);
//
//    return total;
//}

int32_t CTcpServ::send_all(int client_idx, void* buf, int len)
{
    char* data = (char*)buf; // текущий указатель на данные
    int total = 0;  // всего передано байт
    int cnt = 0;    // передано байт в текущей передаче
    int size = len; // осталось передать 
    while (size > 0)
    {
        cnt = ::send(m_client_sd[client_idx], data, size, 0);
        if (cnt == -1)
        {  // Ощибка EAGAIN может возникать при неблокирующем соединении, тогда надо просто повторить
            printf("    CTcpServ::%s: %s (%d)\r", __func__, strerror(errno), errno);
            if(errno != EAGAIN)
                return SOCKET_ERROR;
        }
        else
        {
            total += cnt;
            printf("    CTcpServ::%s: data portion %d of %d (total %d of %d)\n", __func__, cnt, size, total, len);
            data += cnt;
            size -= cnt;
        }
    }

    return total;
}

int32_t CTcpServ::recv_all(int client_idx, void* buf, int len)
{
    char* data = (char*)buf;
    int total = 0;
    int cnt = 0;
    int size = len;
    while (size > 0)
    {
        cnt = ::recv(m_client_sd[client_idx], data, size, 0);
        if (cnt == -1)
        {
            printf(" %s (%d)\n", strerror(errno), errno);
            return SOCKET_ERROR;
        }
        if (cnt == 0)
            return 0; // разрыв соединения
        data += cnt;
        size -= cnt;
        total += cnt;
    }

    return total;
}
