#ifndef _HTTPERR_
#define _HTTPERR_

#include <iostream>
#include <fstream>
#include <map>

#include "ServerConf.hpp"
#include "Config.hpp"

class HTTPError
{
    private :
        static std::map<int, std::string>  _status_codes;

        HTTPError();
        HTTPError(HTTPError const &a);
        ~HTTPError();

        HTTPError   &operator=(HTTPError const &a);
    public :
        static void         initHTTPErrors();
        static std::string  getErrorString(int code);
        static std::string  buildErrorPage(ServerConf const &conf, int &code);
};

#endif