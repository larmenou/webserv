#include "Config.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
        return std::cerr << "Usage : ./parser-tester <.conf path>" << std::endl, 1;
    try {
        Config conf;
        std::string path(av[1]);

        conf.initConfig(path);
    } catch (std::exception &e)
    {
        std::cerr << "Error : " << e.what() << std::endl;
    }
}