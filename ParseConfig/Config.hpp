#ifndef _CONFIG_
#define _CONFIG_

# include <iostream>
# include <vector>
# include <fstream>
# include <sstream>

# include "ServerConf.hpp"

# define BUFFER_SIZE    10
# define SPACES         " \n\t\f\r\v"

void    split(std::string &in, std::vector<std::string> &out, char sep);

class   Config
{
    private :
        std::vector<ServerConf> _servers;
        std::string             _file;
        bool                    _isInit;

    public :
        Config();
        Config(Config const &a);
        Config(std::string &config_path);
        ~Config();
        Config  &operator=(Config const &a);

        void    initConfig(std::string &config_path);
        void    parse();
        void    parseServer(size_t left, size_t right);
        void    updateFromDirParams(std::vector<std::string> &dirs, ServerConf &conf);
};

#endif