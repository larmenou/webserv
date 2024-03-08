#include "Config.hpp"

int main(int ac, char **av)
{
    Config conf;
    std::string path(av[1]);
    std::string host("");
    std::string ip("127.0.0.1");

    if (ac != 2)
        return std::cerr << "Usage : ./parser-tester <.conf path>" << std::endl, 1;
    try {
        conf.initConfig(path);
        std::cout << "Found " << conf.getServerFromHostAndIP(host, ip).getIP() << std::endl;
    } catch (std::exception &e)
    {
        std::cerr << "Error : " << e.what() << std::endl;
    }
}