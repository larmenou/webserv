#ifndef _CONFIG_
#define _CONFIG_

# include <iostream>
# include <vector>
# include <map>
# include <fstream>
# include <sstream>
# include <arpa/inet.h>
# include <cstdlib>

# include "ServerConf.hpp"
# include "Route.hpp"

# define BUFFER_SIZE    10
# define SPACES         " \n\t\f\r\v"

void    split(std::string &in, std::vector<std::string> &out, char sep);

class   Config
{
    private :
        std::vector<ServerConf> _servers;
        std::map<std::string, bool (Config::*)(std::vector<std::string>&, ServerConf&)>   _directiveFuncs;
        std::string             _file;
        bool                    _isInit;

        void                parse();
        void                parseServer(size_t left, size_t right);
        void                updateFromDirParams(std::vector<std::string> &dirs, ServerConf &conf);
        void                extractLocations(std::string &substr, ServerConf &conf);
        Route               parseLocation(std::string &substr, size_t left, size_t right);

        bool                listen(std::vector<std::string> &dirs, ServerConf &conf);
        bool                server_name(std::vector<std::string> &dirs, ServerConf &conf);
        bool                error_page(std::vector<std::string> &dirs, ServerConf &conf);
        bool                client_max_body_size(std::vector<std::string> &dirs, ServerConf &conf);

    public :
        Config();
        Config(Config const &a);
        Config(std::string &config_path);
        ~Config();
        Config  &operator=(Config const &a);

        void    initConfig(std::string &config_path);
};

#endif