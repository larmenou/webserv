#ifndef _REQ_
#define _REQ_

#include <map>
#include <iostream>
#include <sstream>

#include "Config.hpp"

class Request
{
    private :
        std::string                         _urn;
        std::string                         _http_ver;
        long                                _method;
        std::string                         _body;
        std::map<std::string, std::string>  _headers;

        Request();

        void    parseFromRaw(std::string &raw);
        void    parseLineHeader(std::string &line);
    public :
        Request(std::string &raw_req);
        ~Request();

        const std::string   getURN() const;
        const std::string   getHTTPVersion() const;
        const std::string   getBody() const;
        long                getMethod() const;
        const std::map<std::string, std::string>  &getHeaders() const;
};

std::ostream    &operator<<(std::ostream &os, const Request &req);

#endif