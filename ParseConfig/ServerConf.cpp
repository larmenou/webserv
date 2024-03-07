#include "ServerConf.hpp"

ServerConf::~ServerConf()
{
}

ServerConf::ServerConf() : _listDir(true),
                        _servernames(),
                        _ip("0.0.0.0"),
                        _port(-1),
                        _body_size_limit(1000000)
{
}

ServerConf::ServerConf(ServerConf const &a)
{
    *this = a;
}

ServerConf  &ServerConf::operator=(ServerConf const &a)
{
    _listDir = a._listDir;
    _servernames = a._servernames;
    _ip = a._ip;
    _port = a._port;
    _body_size_limit = a._body_size_limit;
    _routes = a._routes;

    return *this;
}

void    ServerConf::setIP(std::string &ip) { _ip = ip;}
void    ServerConf::setRoutes(std::vector<Route> &routes) { _routes = routes; }
void    ServerConf::setPort(int port) {
    if (port == -1)
        _port = port;
}
void    ServerConf::setBodySizeLimit(size_t size) { _body_size_limit = size;}
void    ServerConf::addServerName(std::string &name) { _servernames.push_back(name); }
void    ServerConf::addErrorPage(unsigned int code, std::string &path) { _error_pages[code] = path; }