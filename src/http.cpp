#include "http.hpp"
#include "helper.hpp"
#include "socket.hpp"
#include "param.hpp"
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

std::string HTTP::Reason_phrase(int code) {
    switch (code) {
        case 100: return "Continue";
        case 101: return "Switching Protocols";

        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 203: return "Non-Authoritative Information";
        case 204: return "No Content";
        case 205: return "Reset Content";
        case 206: return "Partial Content";

        case 300: return "Multiple Choices";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 304: return "Not Modified";
        case 305: return "Use Proxy";
        case 307: return "Temporary Redirect";

        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 402: return "Payment Required";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 406: return "Not Acceptable";
        case 407: return "Proxy Authentication Required";
        case 408: return "Request Timeout";
        case 409: return "Conflict";
        case 410: return "Gone";
        case 411: return "Length Required";
        case 412: return "Precondition Failed";
        case 413: return "Content Too Large";
        case 414: return "URI Too Long";
        case 415: return "Unsupported Media Type";
        case 416: return "Range Not Satisfiable";
        case 417: return "Expectation Failed";
        case 426: return "Upgrade Required";

        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        case 505: return "HTTP Version Not Supported";

        default: return std::string();
    }
}

HTTPRequest::HTTPRequest() : type(0) {}

void HTTPRequest::set_type(std::string tp) {
    if (tp == "TEXT")
        type = 1;
    else if (tp == "FILE")
        type = 2;
    else
        type = 0;
}

std::string HTTPRequest::get_type() {
    if (type == 1)
        return "TEXT";
    if (type == 2)
        return "FILE";
    return "UNKNOWN";
}

HTTPResponse::HTTPResponse() : type(0) {}

void HTTPResponse::set_type(std::string tp) {
    if (tp == "TEXT")
        type = 1;
    else if (tp == "FILE")
        type = 2;
    else
        type = 0;
}

std::string HTTPResponse::get_type() {
    if (type == 1)
        return "TEXT";
    if (type == 2)
        return "FILE";
    return "UNKNOWN";
}

void HTTPResponse::set_message(const std::string &message) {
    message_body = message;
    version = "HTTP/1.1";
    status_code = Status_Code::OK;
    header_field["Server"] = "Fancy Chat Room";
    header_field["Content-Type"] = "text/txt";
    header_field["Content-Length"] = std::to_string(message_body.size());
    header_field["Connection"] = "Close";
    set_type("TEXT");
}

void HTTPResponse::set_file(const std::string &file_name) {
    if (file.file_open_read(file_name) < 0) {
        set_notfound();
        return;
    }
    version = "HTTP/1.1";
    status_code = Status_Code::OK;
    header_field["Server"] = "Fancy Chat Room";
    header_field["Content-Type"] = file.type() + "/" + file.extension();
    header_field["Content-Length"] = std::to_string(file.size());
    header_field["Connection"] = "Close";
    set_type("FILE");
}

void HTTPResponse::set_notfound() {
    message_body = "404 Not Found QQ";
    version = "HTTP/1.1";
    status_code = Status_Code::NotFound;
    header_field["Server"] = "Fancy Chat Room";
    header_field["Content-Type"] = "text/txt";
    header_field["Content-Length"] = std::to_string(message_body.size());
    header_field["Connection"] = "Close";
    set_type("TEXT");
}

HTTPSender::HTTPSender(int _fd) : skt() {
    skt.init(_fd);
}

HTTPRequest HTTPSender::request() {
    HTTPRequest req;
    int state = 0;
    char c;
    std::string buf;
    for (int rt; (rt = skt.read(&c, 1)) >= 0;) {
        if (rt > 0)
            buf.push_back(c);
        if (int(buf.size()) >= 2 && buf.substr(buf.size() - 2, 2) == "\r\n") {
            buf.pop_back(), buf.pop_back();
            if (buf.empty()) {
                req.set_type("TEXT"); 
                break;
            }
            if (state == 0) {
                std::stringstream ss(buf);
                ss >> req.method >> req.request_target >> req.version;
                ++state;
            }
            else {
                int cur = 0;
                std::string field_name;
                std::string field_value;
                for (auto c : buf)
                    if (cur == 0) {
                        if (c != ':')
                            field_name.push_back(c);
                        else
                            ++cur;
                    }
                    else {
                        if (c != ' ')
                            cur = 2;
                        else if (cur == 1)
                            continue;
                        field_value.push_back(c);
                    }
                req.header_field[field_name] = field_value;
            }
            buf.clear();
        }
    }
    if (req.get_type() == "UNKNOWN")
        return req;
    if (req.header_field.find("Content-Length") != req.header_field.end()) {
        // Assume that Content-Length will always correct
        // TODO: Reject request if Content-Length is too big
        long long len = stoll(req.header_field["Content-Length"]);
        char buffer[HTTPBUFSIZE];
        // TODO: Write to a temporary file if the content type is file
        while (len > 0) {
            int sz = skt.read(buffer, std::min((long long)HTTPBUFSIZE - 1, len));
            if (sz < 0) {
                req.set_type("UNKNOWN");
                return req;
            }
            buffer[sz] = 0;
            req.message_body += std::string(buffer);
            len -= sz;
        }
    }
    _helper_log(req.method + " " + req.request_target + " " + req.version);
    for (auto [n, v] : req.header_field)
        _helper_log(n + ": " + v);
    _helper_log(req.message_body);
    return req; 
}

int HTTPSender::response(HTTPResponse &res) {
    if (res.get_type() == "UNKNOWN")
        return -1;
    if (skt.write(res.version + " " + std::to_string(int(res.status_code)) + " " + res.Reason_phrase(int(res.status_code)) + "\r\n") < 0)
        return -1;
    for (auto [n, v] : res.header_field)
        if (skt.write(n + ": " + v + "\r\n") < 0)
            return -1;
    if (skt.write("\r\n") < 0)
        return -1;
    if (res.get_type() == "TEXT") {
        if (!res.message_body.empty() && skt.write(res.message_body) < 0)
            return -1;
        return 0;
    } 
    else if (res.get_type() == "FILE") {
        char buffer[HTTPBUFSIZE];
        long long len = res.file.size();
        while (len > 0) {
            long long rt = res.file.readf(buffer, std::min((long long)HTTPBUFSIZE, len));
            if (skt.write(buffer, rt) < 0)
                return -1;
            len -= rt;
        }
        return 0;
    }
    return -1;
}

HTTPServer::HTTPServer(int _port) : port_number(_port), skt() {
    skt.init();
    struct sockaddr_in sockinfo;
    memset(&sockinfo, 0, sizeof sockinfo);
    sockinfo.sin_family = AF_INET;
    sockinfo.sin_port = htons(port_number);
    sockinfo.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(skt.fd, (struct sockaddr *)&sockinfo, sizeof sockinfo) < 0)
        _helper_fail2("bind error", 1);
    if (listen(skt.fd, 1024) < 0)
        _helper_fail2("listen error", 1);
}

HTTPSender* HTTPServer::wait_client() {
    struct sockaddr_in cliaddr;
    int clilen = sizeof(cliaddr);
    int conn_fd = accept(skt.fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
    if (conn_fd < 0) {
        if (errno == EINTR || errno == EAGAIN) 
            return nullptr;
        if (errno == ENFILE) {
            _helper_warning("out of file descriptor table (maxconn " + std::to_string(MAXFD) + ")");
            return nullptr;
        }
        _helper_fail2("accept error", 1);
    }
    HTTPSender* rt = new HTTPSender(conn_fd);
    rt->host = inet_ntoa(cliaddr.sin_addr);
    _helper_log("get new request fd" + std::to_string(conn_fd) + " from " + rt->host);
    return rt;
}
