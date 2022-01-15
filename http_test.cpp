#include "http.hpp"
#include "param.hpp"
#include "helper.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2)
        exit(0);
    HTTPServer server(atoi(argv[1]));
    while (true) {
        HTTPSender* client = server.wait_client();
        HTTPRequest req = client->read_request();
        if (req.get_type() == "UNKNOWN")
            break;
        if (req.request_target == "/")
            req.request_target = "/index.html";
        HTTPResponse res;
        res.set_file(SERVER_PUBLIC_DIR + req.request_target);
        if (client->send_response(res) < 0)
            _helper_warning("response error");
        delete client;
    }
}
