#include "http.hpp"
#include "helper.hpp"
#include "socket.hpp"
#include "param.hpp"
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>

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

HTTPHeader::HTTPHeader() : type(0) {}

void HTTPHeader::set_type(std::string tp) {
    if (tp == "TEXT")
        type = 1;
    else if (tp == "FILE")
        type = 2;
    else if (tp == "ERROR")
        type = 3;
    else
        type = 0;
}

std::string HTTPHeader::get_type() {
    if (type == 1)
        return "TEXT";
    if (type == 2)
        return "FILE";
    if (type == 3)
        return "ERROR";
    return "UNKNOWN";
}

int HTTPHeader::read_header_field(Socket &skt) {
    int rt;
    std::string buf;
    for (char c; (rt = skt.read(&c, 1)) >= 0;) {
        if (rt > 0)
            buf.push_back(c);
        if (int(buf.size()) >= 2 && buf.substr(buf.size() - 2, 2) == "\r\n") {
            buf.pop_back(), buf.pop_back();
            if (buf.empty())
                return 0;
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
            header_field[field_name] = field_value;
            buf.clear();
        }
    }
    return -1;
}

int HTTPHeader::write_header_field(Socket &skt) {
    for (auto [n, v] : header_field)
        if (skt.write(n + ": " + v + "\r\n") < 0)
            return -1;
    if (skt.write("\r\n") < 0)
        return -1;
    return 0;
}

int HTTPHeader::read_content(Socket &skt) {
    if (header_field.find("Content-Length") != header_field.end()) {
        // Assume that Content-Length will always correct
        // TODO: Reject request if Content-Length is too big
        long long len = std::stoll(header_field["Content-Length"]);
        char buffer[HTTPBUFSIZE];
        if (get_type() == "FILE") {
            char tempfile[50];
            #ifdef SERVER
            strcpy(buffer, path_combine(SERVER_PUBLIC_DIR, TEMP_TEMPLATE).c_str());
            #else
            strcpy(buffer, path_combine(CLIENT_PUBLIC_DIR, TEMP_TEMPLATE).c_str());
            #endif
            if (mkstemp(tempfile) < 0) {
                set_type("ERROR");
                return -1;
            }
            if (file.file_open_write(tempfile) < 0) {
                set_type("ERROR");
                return -1;
            }
        }
        while (len > 0) {
            int sz = skt.read(buffer, std::min((long long)HTTPBUFSIZE - 1, len));
            if (sz < 0) {
                set_type("UNKNOWN");
                return -1;
            }
            buffer[sz] = 0;
            if (get_type() == "TEXT") {
                message_body += std::string(buffer);
            }
            else if (get_type() == "FILE") {
                if (file.writef(buffer, sz) < 0) {
                    set_type("ERROR");
                    return -1;
                }
            }
            len -= sz;
        }
    }
    return 0;
}

int HTTPHeader::write_content(Socket &skt) {
    if (get_type() == "TEXT") {
        if (!message_body.empty() && skt.write(message_body) < 0)
            return -1;
    } 
    else if (get_type() == "FILE") {
        char buffer[HTTPBUFSIZE];
        long long len = file.size();
        while (len > 0) {
            long long rt = file.readf(buffer, std::min((long long)HTTPBUFSIZE, len));
            if (skt.write(buffer, rt) < 0)
                return -1;
            len -= rt;
        }
    }
    return 0;
}

HTTPRequest::HTTPRequest() : HTTPHeader() {}

void HTTPRequest::set_message(const std::string &message, const std::string &tar, const std::string &_method) {
    header_field.clear();
    message_body = message;
    method = _method;
    request_target = tar;
    version = "HTTP/1.1";
    header_field["Content-Type"] = "text/plain";
    header_field["Content-Length"] = std::to_string(message_body.size());
    header_field["Connection"] = "keep-alive";
    set_type("TEXT");
}

void HTTPRequest::set_file(const std::string &file_name, const std::string &tar, const std::string &_method) {
    header_field.clear();
    if (file.file_open_read(file_name) < 0) {
        set_type("UNKNOWN");
        return;
    }
    version = "HTTP/1.1";
    method = _method;
    request_target = tar;
    header_field["Content-Type"] = file.type() + "/" + file.extension();
    header_field["Content-Length"] = std::to_string(file.size());
    header_field["Connection"] = "keep-alive";
    set_type("FILE");
}

HTTPResponse::HTTPResponse() : HTTPHeader() {}

void HTTPResponse::set_message(const std::string &message, bool ishtml) {
    header_field.clear();
    message_body = message;
    version = "HTTP/1.1";
    status_code = Status_Code::OK;
    header_field["Server"] = "Fancy Chat Room";
    if (ishtml)
        header_field["Content-Type"] = "text/html";
    else
        header_field["Content-Type"] = "text/plain";
    header_field["Content-Length"] = std::to_string(message_body.size());
    header_field["Connection"] = "Close";
    set_type("TEXT");
}

void HTTPResponse::set_file(const std::string &file_name) {
    header_field.clear();
    if (file.file_open_read(file_name) < 0) {
        set_status(Status_Code::NotFound);
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

void HTTPResponse::set_redirect(const std::string &location) {
    status_code = Status_Code::Found;
    header_field.clear();
    version = "HTTP/1.1";
    header_field["Server"] = "Fancy Chat Room";
    header_field["Location"] = location;
    header_field["Content-Type"] = "text/plain";
    header_field["Content-Length"] = "0";
    header_field["Connection"] = "Close";
    set_type("TEXT");
}

void HTTPResponse::set_status(Status_Code status) {
    header_field.clear();
    status_code = status;
    if (status == Status_Code::NotFound) {
        message_body = "404 Not Found QQ";
    }
    else if (status == Status_Code::MethodNotAllowed) {
        message_body = "405 Method Not Allowed";
    }
    else if (status == Status_Code::InternalServerError) {
        message_body = "500 Internal Server Error";
    }
    else {
        status_code = Status_Code::NotImplemented;
        message_body = "Unexpected Error";
        _helper_warning("Unsupported status code, why you are here?"); 
    }
    version = "HTTP/1.1";
    header_field["Server"] = "Fancy Chat Room";
    header_field["Content-Type"] = "text/plain";
    header_field["Content-Length"] = std::to_string(message_body.size());
    header_field["Connection"] = "Close";
    set_type("TEXT");
}

HTTPSender::HTTPSender(int _fd) : skt() {
    skt.init(_fd);
}

HTTPRequest HTTPSender::read_request() {
    HTTPRequest req;
    int rt;
    std::string buf;
    for (char c; (rt = skt.read(&c, 1)) >= 0;) {
        if (rt > 0)
            buf.push_back(c);
        if (int(buf.size()) >= 2 && buf.substr(buf.size() - 2, 2) == "\r\n") {
            buf.pop_back(), buf.pop_back();
            std::stringstream ss(buf);
            ss >> req.method >> req.request_target >> req.version;
            break;
        }
    }
    if (rt < 0 || req.read_header_field(skt) < 0)
        return req;
    if (req.method == "GET") {
        req.set_type("TEXT");
    }
    else if (req.method == "POST") {
        if (req.request_target.find("upload") != std::string::npos)
            req.set_type("FILE");
        else
            req.set_type("TEXT");
    }
    else {
        req.set_type("UNKNOWN");
    }
    if (req.read_content(skt) < 0) {
        return req;
    }
    _helper_log("read " + req.method + " " + req.request_target + " " + req.version);
    return req; 
}

int HTTPSender::send_request(HTTPRequest &req) {
    if (req.get_type() == "UNKNOWN")
        return -1;
    if (skt.write(req.method + " " + req.request_target + " " + req.version + "\r\n") < 0)
        return -1;
    if (req.write_header_field(skt) < 0)
        return -1;
    if (req.write_content(skt) < 0)
        return -1;
    return 0;
}

HTTPResponse HTTPSender::read_response() {
    HTTPResponse res;
    int rt;
    std::string buf;
    for (char c; (rt = skt.read(&c, 1)) >= 0;) {
        if (rt > 0)
            buf.push_back(c);
        if (int(buf.size()) >= 2 && buf.substr(buf.size() - 2, 2) == "\r\n") {
            buf.pop_back(), buf.pop_back();
            std::stringstream ss(buf);
            std::string status, reason;
            ss >> res.version >> status >> reason;
            res.status_code = Status_Code(std::stoi(status));
            break;
        }
    }
    if (rt < 0 || res.read_header_field(skt) < 0)
        return res;
    if (res.header_field["Content-Type"] == "text/plain") {
        res.set_type("TEXT");
    }
    else {
        res.set_type("FILE");
    }
    if (res.read_content(skt) < 0) {
        return res;
    }
    _helper_log(res.version + " " + std::to_string(int(res.status_code)) + " " + Reason_phrase(int(res.status_code)));
    return res; 
}

int HTTPSender::send_response(HTTPResponse &res) {
    if (res.get_type() == "UNKNOWN")
        return -1;
    if (skt.write(res.version + " " + std::to_string(int(res.status_code)) + " " + res.Reason_phrase(int(res.status_code)) + "\r\n") < 0)
        return -1;
    if (res.write_header_field(skt) < 0)
        return -1;
    if (res.write_content(skt) < 0)
        return -1;
    _helper_log("send " + res.version + " " + std::to_string(int(res.status_code)) + " " + Reason_phrase(int(res.status_code)));
    return 0;
}

HTTPServer::HTTPServer(int _port) : port_number(_port), skt() {
    skt.init();
    _helper_msg("Server listen socket: " + std::to_string(skt.fd));
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
    _helper_log("get new request fd " + std::to_string(conn_fd) + " from " + rt->host);
    return rt;
}
