#ifndef _HTTP_HPP_
#define _HTTP_HPP_

#include <string>

class HTTP {
public:
    enum class Code {
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


#endif // _HTTP_HPP_
