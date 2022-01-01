#include <iostream>
#include <string>
#include "helper.hpp"

void _failed(const std::string &msg, const std::string &name, int line, bool errornum, int exitcode) {
    std::cerr << "\033[1;37m" << name + ":" + std::to_string(line) + ":" + "\033[0m" << " ";
    std::cerr << "\033[1;31m" << "error:" << "\033[0m" << " " << msg << std::endl;
    if (errornum)
        std::cerr << " Error number: " << errno << std::endl;
    exit(exitcode);
}

void _failed(const std::string &msg, const std::string &name, int line, bool errornum) {
    _failed(msg, name, line, errornum, 1);
}

void _failed(const std::string &msg, const std::string &name, int line) {
    _failed(msg, name, line, 0, 1);
}

void _warning(const std::string &msg, const std::string &name, int line, bool errornum) {
    std::cerr << "\033[1;37m" << name + ":" + std::to_string(line) + ":" + "\033[0m" << " ";
    std::cerr << "\033[1;35m" << "warning:" << "\033[0m" << " " << msg << std::endl;
    if (errornum)
        std::cerr << " Error number: " << errno << std::endl;
}

void _warning(const std::string &msg, const std::string &name, int line) {
    _warning(msg, name, line, 0);
}

void _msg(const std::string &msg, const std::string &name, int line) {
    std::cerr << "\033[1;37m" << name + ":" + std::to_string(line) + ":" + "\033[0m" << " ";
    std::cerr << "\033[1;34m" << "note:" << "\033[0m" << " " << msg << std::endl;
}
