#ifndef _SERVERCONF_
# define _SERVERCONF_

#include <iostream>
#include <vector>
#include <map>

#include "Route.hpp"

typedef struct s_route
{
        std::string     _route;
        long            _methodPerms;
        std::string     _root;
        std::pair<std::string, std::string>     _rewrite;
        bool            _dirListing;
        std::string     _isDirFile;
        std::string     _cgiFileExtension;
        bool            _acceptUploads;
        std::string     _fileSavePath;
} t_route;

class ServerConf
{
    private :
        bool                        _listDir;
        std::vector<std::string>    _servernames;
        std::vector<Route>          _routes;
        std::map<unsigned int, std::string> _error_pages;
        std::string                 _ip;
        int                         _port;
        size_t                      _body_size_limit;

    public :
        ServerConf();
        ServerConf(ServerConf const &a);
        ~ServerConf();
        ServerConf  &operator=(ServerConf const &a);


        long                            getPerms() const;
        size_t                          getBodySizeLimit() const;
        const std::string               &getIP() const;
        unsigned int                    getPort() const;
        const std::vector<std::string>  getNames();
    
        void        setIP(std::string &ip);
        void        setRoutes(std::vector<Route> &routes);
        void        setPort(int port);
        void        setBodySizeLimit(size_t size);
        void        addServerName(std::string &name);
        void        addErrorPage(unsigned int code, std::string &path);
  
};

#endif