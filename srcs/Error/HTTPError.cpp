#include "HTTPError.hpp"

std::map<int, std::string> HTTPError::_status_codes;

void    HTTPError::initHTTPErrors()
{
    _status_codes[100] = "Continue";
    _status_codes[101] = "Switching Protocols";
    _status_codes[102] = "Processing"; // WebDAV; RFC 2518
    _status_codes[103] = "Early Hints"; // RFC 8297

    _status_codes[200] = "OK";
    _status_codes[201] = "Created";
    _status_codes[202] = "Accepted";
    _status_codes[203] = "Non-Authoritative Information";
    _status_codes[204] = "No Content";
    _status_codes[205] = "Reset Content";
    _status_codes[206] = "Partial Content";
    _status_codes[207] = "Multi-Status"; // WebDAV; RFC 4918
    _status_codes[208] = "Already Reported"; // WebDAV; RFC 5842
    _status_codes[226] = "IM Used"; // RFC 3229

    _status_codes[300] = "Multiple Choices";
    _status_codes[301] = "Moved Permanently";
    _status_codes[302] = "Found";
    _status_codes[303] = "See Other";
    _status_codes[304] = "Not Modified";
    _status_codes[305] = "Use Proxy";
    _status_codes[306] = "Switch Proxy";
    _status_codes[307] = "Temporary Redirect";
    _status_codes[308] = "Permanent Redirect"; // RFC 7538

    _status_codes[400] = "Bad Request";
    _status_codes[401] = "Unauthorized";
    _status_codes[402] = "Payment Required";
    _status_codes[403] = "Forbidden";
    _status_codes[404] = "Not Found";
    _status_codes[405] = "Method Not Allowed";
    _status_codes[406] = "Not Acceptable";
    _status_codes[407] = "Proxy Authentication Required";
    _status_codes[408] = "Request Timeout";
    _status_codes[409] = "Conflict";
    _status_codes[410] = "Gone";
    _status_codes[411] = "Length Required";
    _status_codes[412] = "Precondition Failed";
    _status_codes[413] = "Payload Too Large";
    _status_codes[414] = "URI Too Long";
    _status_codes[415] = "Unsupported Media Type";
    _status_codes[416] = "Range Not Satisfiable";
    _status_codes[417] = "Expectation Failed";
    _status_codes[418] = "I'm a teapot"; // RFC 2324
    _status_codes[421] = "Misdirected Request";
    _status_codes[422] = "Unprocessable Entity"; // WebDAV; RFC 4918
    _status_codes[423] = "Locked"; // WebDAV; RFC 4918
    _status_codes[424] = "Failed Dependency"; // WebDAV; RFC 4918
    _status_codes[425] = "Too Early"; // RFC 8470
    _status_codes[426] = "Upgrade Required";
    _status_codes[428] = "Precondition Required"; // RFC 6585
    _status_codes[429] = "Too Many Requests"; // RFC 6585
    _status_codes[431] = "Request Header Fields Too Large"; // RFC 6585
    _status_codes[451] = "Unavailable For Legal Reasons"; // RFC 7725

    // Server Error
    _status_codes[500] = "Internal Server Error";
    _status_codes[501] = "Not Implemented";
    _status_codes[502] = "Bad Gateway";
    _status_codes[503] = "Service Unavailable";
    _status_codes[504] = "Gateway Timeout";
    _status_codes[505] = "HTTP Version Not Supported";
    _status_codes[506] = "Variant Also Negotiates"; // RFC 2295
    _status_codes[507] = "Insufficient Storage"; // WebDAV; RFC 4918
    _status_codes[508] = "Loop Detected"; // WebDAV; RFC 5842
    _status_codes[510] = "Not Extended"; // RFC 2774
    _status_codes[511] = "Network Authentication Required"; // RFC 6585
}

static void buildHead(std::string &ret)
{
    ret += "<!DOCTYPE HTML>\n";
    ret += "<html>\n";
    ret += "<head>\n";
    ret += "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n";
    ret += "<title>Error response</title>\n";
    ret += "</head>\n";
}

std::string HTTPError::buildErrorPage(ServerConf const &conf, int code)
{
    std::string ret;

    try
    {
        std::ifstream fs(conf.getErrorPage(code).c_str());

        if (!fs.is_open())
            throw std::exception();
        readAllFile(fs, ret);
        return ret;
    }
    catch (...)
    {
        buildHead(ret);
        ret += "<body>";
        ret += HTTPError::getErrorString(code);
        ret += "</body>\n</html>";
    }
    return ret;
}

std::string HTTPError::getErrorString(int code)
{
    std::map<int, std::string>::iterator ite;

    if ((ite = _status_codes.find(code)) == _status_codes.end())
        return "Unkown error code";
    return ite->second;
}