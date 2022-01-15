#include "socket.hpp"
#include "helper.hpp"
#include <sys/socket.h>
#include <sys/poll.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <cstring>

Socket::Socket() : fd(-1) {}

Socket::~Socket() {
    if (fd >= 0)
        close(fd);
}

int Socket::init() {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    return fd;
}

int Socket::init(int _fd) {
/*
    TODO:
    if (fd is not a valid socket)
        _helper_warning("init socket with fd " + std::to_string(fd) + ", which is not a socket");
*/
    fd = _fd;
    return fd;
}

int Socket::read(void *buf, size_t count) {
    while (1) {
        struct pollfd pfds[1];
        pfds[0].fd = fd;
        pfds[0].events = POLLIN;
        poll(pfds, 1, -1);
        if (pfds[0].revents & POLLIN)
            break;
    }
    int num = recv(fd, buf, count, MSG_DONTWAIT);
    if (num <= 0) {
        if (errno == EINTR || errno == EAGAIN) {
            errno = 0;
            return 0;
        }
        _helper_warning2("fd " + std::to_string(fd) + " reading error", 1);
        close(fd);
        fd = -1;
    }
    return num;
}

int Socket::write(void *buf, size_t count) {
    int num = send(fd, buf, count, 0); // MSG_DONTWAIT);
    if (num < 0) {
        if (errno == EINTR || errno == EAGAIN) {
            errno = 0;
            return 0;
        }
        _helper_warning2("fd " + std::to_string(fd) + " writing error", 1);
        close(fd);
        fd = -1;
    }
    return num;
}

int Socket::write(const std::string &msg) {
    char buffer[SOCKBUFSIZE];
    int cur = 0;
    while (cur < int(msg.size())) {
        int nw = std::min(int(msg.size()) - cur, SOCKBUFSIZE);
        memcpy(buffer, msg.c_str() + cur, nw);
        int rt = write(buffer, nw);
        if (rt < 0)
            return rt;
        cur += rt;
    }
    _helper_msg("written " + msg);
    return cur;
}
