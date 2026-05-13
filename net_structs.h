//=************************* net structs *************************

#ifndef _NET_STRUCTS_H_
#define _NET_STRUCTS_H_

struct net_header_t {
    uint32_t type;
    uint32_t size;
};

enum NetCmd {
    ncHello = 0,     //
    ncUssev = 1,     //
    ncAdc = 2,     //
    ncRpu = 3,     //
    ncConsole = 4,     //
    ncExit = 8       //
};

#endif // _NET_STRUCTS_H_
