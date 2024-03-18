#ifndef _REQ_
#define _REQ_

#include <map>
#include <iostream>
#include <sstream>

#include "Config.hpp"

bool    getlineCRLF(std::stringstream &ss, std::string &str);
void    trimstr(std::string &str);

class Request
{
    private :
        std::string                         _urn;
        std::string                         _http_ver;
        long                                _method;
        std::string                         _body;
        std::map<std::string, std::string>  _headers;
        std::map<std::string, std::string>  _getParams;

        Request();

        void    parseFromRaw(std::string &raw);
        void    parseLineHeader(std::string &line);
        void    extractGETParams();

    public :
        Request(std::string &raw_req);
        ~Request();

        long                getMethod() const;
        const std::string   getURN() const;
        const std::string   getHTTPVersion() const;
        const std::string   getBody() const;
        const std::string   findHeader(std::string key) const;
        const std::map<std::string, std::string>  &getHeaders() const;
        const std::map<std::string, std::string>  &getURLParams() const;

        bool    checkExtension(std::string extension) const;
};

std::ostream    &operator<<(std::ostream &os, const Request &req);

#endif