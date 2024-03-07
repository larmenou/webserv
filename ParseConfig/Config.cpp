#include "Config.hpp"

Config::Config()
{

}

Config::~Config()
{

}

void    split(std::string &in, std::vector<std::string> &out, char sep)
{
    size_t end;
    size_t  start = 0;
    do 
    {
        end = in.find(sep, start);
        out.push_back(in.substr(start, end - start));
        start = end + 1;
    } while (end != std::string::npos);
}

static void readAllFile(std::ifstream &fs, std::string &out)
{
    std::string buff;

    while (std::getline(fs, buff))
        out += buff + "\n";
}

void    Config::initConfig(std::string &configPath)
{
    std::ifstream   fs(configPath.c_str());

    if (fs.fail())
        throw std::runtime_error("Could not open configuration file");
    readAllFile(fs, _file);
    parse();
}

static void determineBracketBounds(std::string &str,
                                    size_t start,
                                    size_t &left,
                                    size_t  &right
                                    ,char lb, char rb)
{
    size_t  c = 1;

    left = str.find_first_not_of(SPACES, start);
    if (left == std::string::npos || str[left] != lb)
        throw std::runtime_error("Invalid syntax");
    right = left + 1;
    while (c != 0 && right != str.size())
    {
        if (str[right] == lb)
            c++;
        if (str[right] == rb)
            c--;
        right++;
    }
    if (right == str.size())
        throw std::runtime_error("Invalid syntax");
}

void    Config::parse()
{
    std::size_t server_idx;
    size_t left = 0, right;

    while (true)
    {
        server_idx = _file.find("server", left);
        if (server_idx == std::string::npos)
            break;
        server_idx += 6;
        determineBracketBounds(_file, server_idx, left, right, '{', '}');
        parseServer(left, right);
        left = right + 1;
    }
}

static void extractParameters(std::string &config, std::vector<std::string> &parameters)
{
    std::stringstream   ss(config);
    std::string         word;

    while (ss >> word)
        parameters.push_back(word);
}

Route   Config::parseLocation(std::string &substr, size_t left, size_t right)
{
    std::vector<std::string>    directives;
    std::vector<std::string>    parameters;
    std::string                 locationSubstr;
    t_route                     route;

    locationSubstr = substr.substr(left + 1, right - left - 2);
    split(locationSubstr, directives, ';');
    for (size_t i = 0; i < directives.size(); i++)
    {
        extractParameters(directives[i], parameters);
        parameters.clear();
    }
    Route ret(route._methodPerms,
                route._root,
                route._rewrite,
                route._dirListing,
                route._isDirFile,
                route._cgiFileExtension,
                route._acceptUploads,
                route._fileSavePath);
    return ret;
}

void  Config::extractLocations(std::string &substr, ServerConf &conf)
{
    std::size_t         location_idx;
    std::vector<Route>  locations;
    size_t              left = 0, right;
    std::string         route;

    while (true)
    {
        location_idx = substr.find("location");
        if (location_idx == std::string::npos)
            break;
        size_t route_idx = substr.find_first_not_of(SPACES, location_idx + 8);
        size_t route_end = substr.find_first_of(SPACES, route_idx);
        route = substr.substr(route_idx, route_end - route_idx);
        determineBracketBounds(substr, route_end, left, right, '{', '}');
        locations.push_back(parseLocation(substr, left, right));
        substr.erase(location_idx, right - location_idx);
    }
    conf.setRoutes(locations);
}


void    Config::parseServer(size_t left, size_t right)
{
    std::vector<std::string>    directives;
    std::vector<std::string>    parameters;
    std::string                 serverSubstr;
    ServerConf                  conf;

    serverSubstr = _file.substr(left + 1, right - left - 2);
    extractLocations(serverSubstr, conf);
    split(serverSubstr, directives, ';');
    for (size_t i = 0; i < directives.size(); i++)
    {
        extractParameters(directives[i], parameters);
        updateFromDirParams(parameters, conf);
        parameters.clear();
    }
    _servers.push_back(conf);
}

void    Config::updateFromDirParams(std::vector<std::string> &dirs, ServerConf &conf)
{
    (void) conf; (void) dirs;
    /*TODO*/
}

int main()
{
    Config conf;
    std::string filepath("tests/server.conf");
    conf.initConfig(filepath);
}