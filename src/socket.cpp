#include "socket.hpp"
#include "helper.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string>

Socket::Socket() : fd(-1) {}

Socket::~Socket() {
    if (fd >= 0)
        close(fd);
}

int Socket::init() {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    return fd;
}

int Socket::read(void *buf, size_t count) {
    int num = recv(fd, buf, count, MSG_DONTWAIT);
    if (num <= 0) {
        if (errno == EINTR || errno == EAGAIN) {
            errno = 0;
            return 0;
        }
        _helper_warning2("fd" + std::to_string(fd) + " reading error", 1);
        close(fd);
        fd = -1;
    }
    return num;
}

int Socket::write(void *buf, size_t count) {
    int num = send(fd, buf, count, MSG_DONTWAIT);
    if (num < 0) {
        if (errno == EINTR || errno == EAGAIN) {
            errno = 0;
            return 0;
        }
        _helper_warning2("fd" + std::to_string(fd) + " writing error", 1);
        close(fd);
        fd = -1;
    }
    return num;
}
