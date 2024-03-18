#ifndef _CGI_
#define _CGI_

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <sys/wait.h>
#include "Request.hpp"
#include "Route.hpp"

class CGI
{
    private :
        const Request       *_request;
        const Route         *_route;
        std::map<std::string, std::string>  _env;
        std::map<std::string, std::string>  _headers;
        std::string         _body;
        std::string         _cgi_path;
        std::string         _raw_response;
        char                **_env_execve;

        CGI(CGI const &a);
        CGI  &operator=(CGI const &a);

        void    getPathInfo();
        void    getQueryString();
        void    getContentLength();
        void    getContentType();
        void    getRequestMethod();
        void    getServerName(const ServerConf &server);
        void    parentProc(int fds[2], pid_t pid);
        void    childProc(int fds[2]);
        void    parseRaw();
        char    **buildEnvFromAttr();

    public :
        ~CGI();
        CGI();

        void        prepare(Request const &req,
                        Route const &route,
                        ServerConf const &server,
                        std::string remoteaddr);
        void        forwardReq();
        void        setCGI(std::string cgiPath);
        const std::string   &getRawResp() const;
        const std::string   &getBody() const;
        const std::map<std::string, std::string> &getHeaders() const;
};

#endif