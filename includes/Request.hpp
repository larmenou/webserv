#ifndef _REQ_
#define _REQ_

#include <map>
#include <iostream>
#include <sstream>

#include "Config.hpp"

#define MAX_HEADER 16000

bool    getlineCRLF(std::stringstream &ss, std::string &str);
void    trimstr(std::string &str);
void    strtolower(std::string &str);

class Request
{
    private :
        std::string                         _urn;
        std::string                         _http_ver;
        long                                _method;
        std::string                         _body;
        std::string                         _raw_header;
        std::map<std::string, std::string>  _headers;
        ssize_t                             _content_length;

        std::string                         _query_string;
        bool                                _isParsed;
        bool                                _keep_alive;

        Request(Request const &a);
        Request &operator=(Request const &a);

        void    parseFromRaw(std::string raw);
        void    parseLineHeader(std::string &line);
        void    extractGETParams();

    public :
        Request();
        ~Request();

        ssize_t             getContentLength() const;
        bool                isKeepAlive() const;
        long                getMethod() const;
        const std::string   getURN() const;
        const std::string   getHTTPVersion() const;
        const std::string   getBody() const;
        bool                isParsed() const;
        const std::string   findHeader(std::string key) const;
        const std::map<std::string, std::string>  &getHeaders() const;
        const std::string   &getURLParams() const;

        const std::string   &respHeader();
        const std::string   &respBody();

        bool                checkExtension(std::string extension) const;
        size_t              receive_header(const char *chunk, size_t pkt_len);
        void                reset();
};

std::ostream    &operator<<(std::ostream &os, const Request &req);

#endif