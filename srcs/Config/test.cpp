#include "Config.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
        return std::cerr << "Usage : ./parser-tester <.conf path>" << std::endl, 1;
    try {
        Config conf;
        std::string path(av[1]);
        std::string host("");
        std::string ip("127.0.0.1");
        conf.initConfig(path);
        const ServerConf &server = conf.getServerFromHostAndIP(host, ip);

        std::cout << "Found " << server.getIP()  << ":" << server.getPort() << std::endl;
        std::cout << "404 error page at '" << server.getErrorPage(404) << "'" << std::endl;
        server.getErrorPage(500);
    } catch (std::exception &e)
    {
        std::cerr << "Error : " << e.what() << std::endl;
    }
}