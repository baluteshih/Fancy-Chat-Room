#include <iostream>
#include "helper.hpp"
#include "http.hpp"
#include "socket.hpp"

int main() {
    Helper::warning("this is just a testing", __INFO__);
    Helper::msg("debug testing", __INFO__); // make test DEBUG=1
    std::cout << "Hello World\n";
    Helper::fail("this is just a testing", __INFO__, true, 0);
}
