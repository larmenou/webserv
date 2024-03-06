#ifndef _SERVERCONF_
# define _SERVERCONF_

#include <iostream>
#include <vector>

# define GET        0b1
# define POST       0b10
# define PUT        0b100
# define OPTIONS    0b1000
# define HEAD       0b10000
# define CONNECT    0b100000
# define DELETE     0b1000000
# define PATCH      0b10000000
# define TRACE      0b100000000

class ServerConf
{
    private :
        bool                        _listDir;
        std::vector<std::string>    _servernames;
        std::string                 _ip;
        unsigned int                _port;
        size_t                      _body_size_limit;
        long                         _methodPerms;

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
    
        void        setIP(std::string ip);
        void        setPort(unsigned int port);
        void        setPerms(long perms);
        void        setBodySizeLimit(size_t size);
        void        addServerName(std::string &name);
  
};

#endif