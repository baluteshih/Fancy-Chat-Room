#ifndef _SOCKET_HPP_
#define _SOCKET_HPP_

#include <cstddef>
#include <string>
using std::size_t;

const int SOCKBUFSIZE = 65536;

class Socket {
    int fd;
public:
    friend class HTTPServer;
    friend class HTTPSender;
    Socket();
    ~Socket();
    int init();
    int init(int _fd);
    int read(void *buf, size_t count);
    int write(void *buf, size_t count);
    int write(const std::string &msg);
};

#endif // _SOCKET_HPP_
