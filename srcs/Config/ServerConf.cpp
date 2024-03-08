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

const std::string               ServerConf::getErrorPage(unsigned int code) const
{
    std::map<unsigned int, std::string>::const_iterator   ite;

    ite = _error_pages.find(code);
    if (_error_pages.find(code) == _error_pages.end())
        throw std::runtime_error("Could not find error page.");
    return ite->second;
}


bool    ServerConf::hasServername(std::string &name) const
{
    if (std::find(_servernames.begin(), _servernames.end(), name) != _servernames.end())
        return true;
    return false;
}

const Route &ServerConf::findRouteFromURN(std::string &urn) const
{
    if (_routes.size() == 0)
        throw std::runtime_error("No route");
    for (size_t i = 0; i < _routes.size(); i++)
    {
        const std::string &route = _routes[i].getRoute();
        std::cout << urn.substr(0, route.length()) << std::endl;
        if (route.compare(urn.substr(0, route.length())) == 0)
            return _routes[i];
    }
    throw std::runtime_error("No route from URN");
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