#include "helper.hpp"
#include "main_thread.hpp"
#include <iostream>

Main_Thread main_thread;

void polling_handler(HTTPServer &server) {
    while (true) {
        HTTPSender *connection = server.wait_client();
        create_thread(client_handler, connection);
    } 
}

int main(int argc, char *argv[]) {
    random_init();
    if (argc < 2)
        help(argv[0]);
    HTTPServer server(atoi(argv[1]));
    create_thread(polling_handler, std::ref(server));
    // possibly race condition, how to solve it?
    main_thread.start();
}
