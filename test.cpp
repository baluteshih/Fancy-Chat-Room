#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "helper.hpp"
#include "http.hpp"
#include "socket.hpp"
#include "param.hpp"

int main() {
    _helper_warning("this is just a testing");
    _helper_msg("debug testing"); // make test DEBUG=1
    std::cout << "Hello World\n";
    char buff[50];
    strcpy(buff, (SERVER_PUBLIC_DIR + "/" + TEMP_TEMPLATE).c_str());
    std::cout << "mktemp result = " << mkstemp(buff) << "\n";
    std::cout << "temp filename = " << buff << "\n";
    std::cout << "remove result = " << remove(buff) << "\n";
    _helper_fail3("this is just a testing", true, 0);
}
