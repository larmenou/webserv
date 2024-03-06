#include "ServerConf.hpp"

ServerConf::~ServerConf()
{
}

ServerConf::ServerConf() : _listDir(true),
                        _servernames(),
                        _ip("0.0.0.0"),
                        _port(80),
                        _body_size_limit(1000000),
                        _methodPerms(GET | POST)
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
    _methodPerms = a._methodPerms;

    return *this;
}