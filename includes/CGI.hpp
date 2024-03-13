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
        std::string         _cgi_path;
        std::map<std::string, std::string>  _env;

        CGI(CGI const &a);
        CGI  &operator=(CGI const &a);

        void getPathInfo();
        void getQueryString();
        void getContentLength();
        void getContentType();
        void getRequestMethod();
        void getServerName(const ServerConf &server);

    public :
        ~CGI();
        CGI();

        void        prepare(Request const &req,
                        Route const &route,
                        ServerConf const &server,
                        std::string remoteaddr);
        std::string forwardReq();
        char        **buildEnvFromAttr();
        void        setCGI(std::string cgiPath);
};

#endif