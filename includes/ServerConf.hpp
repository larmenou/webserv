#ifndef _SERVERCONF_
# define _SERVERCONF_

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

#include "Route.hpp"

class ServerConf
{
    private :
        bool                        _listDir;
        std::vector<std::string>    _servernames;
        std::set<Route>             _routes;
        Route                       _default_route;
        std::map<unsigned int, std::string> _error_pages;
        std::string                 _root;
        std::string                 _ip;
        int                         _port;
        size_t                      _body_size_limit;

    public :
        ServerConf();
        ServerConf(ServerConf const &a);
        ~ServerConf();
        ServerConf  &operator=(ServerConf const &a);

        size_t                          getBodySizeLimit() const;
        const std::string               &getIP() const;
        int                             getPort() const;
        const std::vector<std::string>  &getNames() const;
        const std::string               &getErrorPage(unsigned int code) const;
        const std::string               &getRoot() const;
        const Route                     &getDefaultRoute() const;

        bool        hasServername(std::string name) const;
        const Route &findRouteFromURN(std::string urn) const;

        void        setIP(std::string ip);
        void        setRoutes(std::set<Route> &routes);
        void        setPort(int port);
        void        setBodySizeLimit(size_t size);
        void        addServerName(std::string name);
        void        addErrorPage(unsigned int code, std::string path);
        void        setRoot(std::string root);
        void        setIndex(std::string index);
        void        setDirListing(bool state);
};

std::ostream    &operator<<(std::ostream &os, const ServerConf &conf);

#endif