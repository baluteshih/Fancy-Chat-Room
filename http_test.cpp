#include "http.hpp"
#include "param.hpp"
#include "helper.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2)
        exit(0);
    HTTPServer server(atoi(argv[1]));
    while (true) {
        HTTPSender* client = server.wait_client();
        _helper_msg("Here");
        HTTPRequest req = client->request();
        if (req.get_type() == "UNKNOWN")
            break;
        if (req.request_target == "/")
            req.request_target = "/index.html";
        HTTPResponse res;
        res.set_file(CLIENT_PUBLIC_DIR + req.request_target);
        if (client->response(res) < 0)
            _helper_warning("response error");
        delete client;
    }
}
