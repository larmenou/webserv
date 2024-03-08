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
    _error_pages = a._error_pages;

    return *this;
}

size_t                          ServerConf::getBodySizeLimit() const { return _body_size_limit; }
const std::string               &ServerConf::getIP() const { return _ip; }
int                             ServerConf::getPort() const { return _port; }
const std::vector<std::string>  &ServerConf::getNames() const { return _servernames; }

bool    ServerConf::hasServername(std::string &name) const
{
    if (std::find(_servernames.begin(), _servernames.end(), name) != _servernames.end())
        return true;
    return false;
}

void    ServerConf::setIP(std::string &ip) { _ip = ip;}
void    ServerConf::setRoutes(std::vector<Route> &routes) { _routes = routes; }
void    ServerConf::setPort(int port) {
    if (_port == -1)
        _port = port;
}
void    ServerConf::setBodySizeLimit(size_t size) { _body_size_limit = size;}
void    ServerConf::addServerName(std::string &name) { _servernames.push_back(name); }
void    ServerConf::addErrorPage(unsigned int code, std::string &path) { _error_pages[code] = path; }

std::ostream    &operator<<(std::ostream &os, ServerConf &conf)
{
    os << "Server on " << conf.getIP() << ":" << conf.getPort();
    if (conf.getNames().size())
    {
        os << "\n\t- server_names :";
        for (size_t i = 0; i < conf.getNames().size(); i++)
            os << "'" << conf.getNames()[i] << "'" << (i != conf.getNames().size() - 1 ? ", " : "");
    }
    os << "\n\t- client body size limit at " <<  conf.getBodySizeLimit();

    return os;
}