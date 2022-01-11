#include <iostream>
#include "helper.hpp"
#include "http.hpp"
#include "socket.hpp"

int main() {
    _helper_warning("this is just a testing");
    _helper_msg("debug testing"); // make test DEBUG=1
    std::cout << "Hello World\n";
    _helper_fail3("this is just a testing", true, 0);
}
