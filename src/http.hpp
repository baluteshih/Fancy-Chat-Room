#ifndef _HTTP_HPP_
#define _HTTP_HPP_

#include <string>
#include <unordered_map>
#include "socket.hpp"
#include "file.hpp"

const int HTTPBUFSIZE = 65536;

class HTTP {
public:
    enum class Status_Code {
        Continue           = 100,
        SwitchingProtocols = 101,

        OK                          = 200,
        Created                     = 201,
        Accepted                    = 202,
        NonAuthoritativeInformation = 203,
        NoContent                   = 204,
        ResetContent                = 205,
        PartialContent              = 206,

        MultipleChoices   = 300,
        MovedPermanently  = 301,
        Found             = 302,
        SeeOther          = 303,
        NotModified       = 304,
        UseProxy          = 305,
        TemporaryRedirect = 307,

        BadRequest                  = 400,
        Unauthorized                = 401,
        PaymentRequired             = 402,
        Forbidden                   = 403,
        NotFound                    = 404,
        MethodNotAllowed            = 405,
        NotAcceptable               = 406,
        ProxyAuthenticationRequired = 407,
        RequestTimeout              = 408,
        Conflict                    = 409,
        Gone                        = 410,
        LengthRequired              = 411,
        PreconditionFailed          = 412,
        ContentTooLarge             = 413,
        PayloadTooLarge             = 413,
        URITooLong                  = 414,
        UnsupportedMediaType        = 415,
        RangeNotSatisfiable         = 416,
        ExpectationFailed           = 417,
        UpgradeRequired             = 426,

        InternalServerError           = 500,
        NotImplemented                = 501,
        BadGateway                    = 502,
        ServiceUnavailable            = 503,
        GatewayTimeout                = 504,
        HTTPVersionNotSupported       = 505
    };
    std::string Reason_phrase(int code);
};

class HTTPHeader : public HTTP {
private:
    int type;
public:
    std::string version;
    std::unordered_map<std::string, std::string> header_field;
    std::string message_body;
    File file;
    void set_type(std::string tp);
    std::string get_type();
    int read_header_field(Socket &skt);
    int write_header_field(Socket &skt);
    int read_content(Socket &skt);
    int write_content(Socket &skt);
    explicit HTTPHeader();
};

class HTTPRequest : public HTTPHeader {
private:
public:
    std::string method;
    std::string request_target;
    explicit HTTPRequest();
    void set_message(const std::string &message, const std::string &tar, const std::string &_method, const std::string &cookie = "");
    void set_file(const std::string &file_name, const std::string &tar, const std::string &_method = "POST", const std::string &cookie = "");
};

class HTTPResponse : public HTTPHeader {
private:
public:
    Status_Code status_code;
    explicit HTTPResponse();
    void set_message(const std::string &message, bool ishtml = false);
    void set_file(const std::string &file_name);
    void set_redirect(const std::string &location);
    void set_predirect(const std::string &location);
    void set_status(Status_Code status);
};

class HTTPSender : public HTTP {
    Socket skt;
public:
    std::string host;
    HTTPSender(int _fd);
    HTTPRequest read_request();
    int send_request(HTTPRequest &req);
    HTTPResponse read_response();
    int send_response(HTTPResponse &res);
};

class HTTPServer : public HTTP {
    int port_number;
    Socket skt;
public:
    HTTPServer(int _port);
    HTTPSender* wait_client();
};

#endif // _HTTP_HPP_
