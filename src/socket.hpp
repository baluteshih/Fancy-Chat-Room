#ifndef _SOCKET_HPP_
#define _SOCKET_HPP_

#include <cstddef>
using std::size_t;

class Socket {
    int fd;
public:
    Socket();
    ~Socket();
    int init();
    int read(void *buf, size_t count);
    int write(void *buf, size_t count);
};

#endif // _SOCKET_HPP_
