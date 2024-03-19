#include "Config.hpp"

std::map<std::string, long> Config::str2permmap;

Config::Config(std::string config_path)
{
    serverUpdateFuncs["listen"] = &Config::listen;
    serverUpdateFuncs["server_name"] = &Config::server_name;
    serverUpdateFuncs["error_page"] = &Config::error_page;
    serverUpdateFuncs["client_max_body_size"] = &Config::client_max_body_size;
    serverUpdateFuncs["root"] = &Config::server_root;
    serverUpdateFuncs["index"] = &Config::server_index;
    serverUpdateFuncs["autoindex"] = &Config::server_autoindex;

    routeUpdateFuncs["root"] = &Config::root;
    routeUpdateFuncs["rewrite"] = &Config::rewrite;
    routeUpdateFuncs["allowed_methods"] = &Config::allowed_methods;
    routeUpdateFuncs["autoindex"] = &Config::autoindex;
    routeUpdateFuncs["index"] = &Config::dir_default;
    routeUpdateFuncs["cgi_extension"] = &Config::cgi_extension;
    routeUpdateFuncs["uploads"] = &Config::save_path;

    str2permmap["GET"] = GET;
    str2permmap["POST"] = POST;
    str2permmap["PUT"] = PUT;
    str2permmap["OPTIONS"] = OPTIONS;
    str2permmap["HEAD"] = HEAD;
    str2permmap["CONNECT"] = CONNECT;
    str2permmap["DELETE"] = DELETE;
    str2permmap["PATCH"] = PATCH;
    str2permmap["TRACE"] = TRACE;

    initConfig(config_path);
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

long         Config::str2perm(std::string &method_str)
{
        std::map<std::string, long>::const_iterator ite;

        ite = str2permmap.find(method_str);
        if (ite == str2permmap.end())
            return -1;
        return ite->second;
}

std::string Config::perm2str(long perm)
{
    std::map<std::string, long>::const_iterator ite = str2permmap.begin();
    for (; ite != str2permmap.end(); ite++)
        if (ite->second == perm)
            return ite->first;
    throw std::runtime_error("Invalid permissions");
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
    size_t server_idx;
    size_t left = 0, right;

    while (true)
    {
        server_idx = _file.find_first_not_of(SPACES, left);
        if (server_idx == std::string::npos)
            break;
        if (_file.substr(server_idx, 6).compare("server") != 0)
            throw std::runtime_error("Found unknown directive.");
        server_idx += 6;
        determineBracketBounds(_file, server_idx, left, right, '{', '}');
        parseServer(left, right);
        left = right + 1;
    }
    if (_servers.size() == 0)
        throw std::runtime_error("No server found in config file.");
}

static void extractParameters(std::string &config, std::vector<std::string> &parameters)
{
    std::stringstream   ss(config);
    std::string         word;

    while (ss >> word)
        parameters.push_back(word);
}

 void   Config::updateRouteFromDirective(std::vector<std::string> &params, Route &route)
 {
    std::map<std::string, bool (Config::*)(std::vector<std::string>&, Route&)>::iterator ite;

    if (params.size() == 0)
        return ;
    ite = routeUpdateFuncs.find(params[0]);
    if (ite == routeUpdateFuncs.end())
        throw std::runtime_error("Unknown location directive.");
    if (((this)->*(ite->second))(params, route) == false)
        throw std::runtime_error("Invalid arguments for location directive.");
 }


void    Config::parseLocation(std::string &substr, size_t left, size_t right, Route &route)
{
    std::vector<std::string>    directives;
    std::vector<std::string>    parameters;
    std::string                 locationSubstr;
    
    locationSubstr = substr.substr(left + 1, right - left - 2);
    split(locationSubstr, directives, ';');
    for (size_t i = 0; i < directives.size(); i++)
    {
        extractParameters(directives[i], parameters);
        updateRouteFromDirective(parameters, route);
        parameters.clear();
    }
}

void  Config::extractLocations(std::string &substr, ServerConf &conf)
{
    std::size_t         location_idx;
    std::set<Route>     locations;
    size_t              left = 0, right;
    std::string         route_str;
    Route               route;


    while (true)
    {
        location_idx = substr.find("location");
        if (location_idx == std::string::npos)
            break;
        size_t route_idx = substr.find_first_not_of(SPACES, location_idx + 8);
        size_t route_end = substr.find_first_of(SPACES, route_idx);
        route_str = substr.substr(route_idx, route_end - route_idx);
        determineBracketBounds(substr, route_end, left, right, '{', '}');
        parseLocation(substr, left, right, route);
        route.setRoute(route_str);
        if (DEBUG)
            std::cout << "[LOG] Adding " << route << std::endl;
        locations.insert(route);
        route = Route();
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
        updateServerFromDirective(parameters, conf);
        parameters.clear();
    }
    if (conf.getPort() == -1)
        throw std::runtime_error("Listening port was not initialized");
    if (DEBUG)
        std::cout << "[LOG] To " << conf << std::endl;
    _servers.push_back(conf);
}

void    Config::updateServerFromDirective(std::vector<std::string> &params, ServerConf &conf)
{
    std::map<std::string, bool (Config::*)(std::vector<std::string>&, ServerConf&)>::iterator ite;

    if (params.size() == 0)
        return ;
    ite = serverUpdateFuncs.find(params[0]);
    if (ite == serverUpdateFuncs.end())
        throw std::runtime_error("Unknown directive.");
    if (((this)->*(ite->second))(params, conf) == false)
        throw std::runtime_error("Invalid arguments for directive.");
}

bool    Config::server_name(std::vector<std::string> &dirs, ServerConf &conf)
{
    if (dirs.size() == 1)
        return false;
    for (size_t i = 1; i < dirs.size(); i++)
        conf.addServerName(dirs[i]);
    return true;
}


bool    Config::listen(std::vector<std::string> &dirs, ServerConf &conf)
{
    std::vector<std::string>    out;
    long                        port;
    char                        *endptr;

    if (dirs.size() != 2)
        return false;
    if (dirs[1].find(':') != std::string::npos)
    {
        split(dirs[1], out, ':');
        if (out.size() != 2)
            return false;
        if (inet_addr(out[0].c_str()) == (in_addr_t)(-1))
            return false;
        port = std::strtol(out[1].c_str(), &endptr, 10);
        if (*endptr != 0 || port < 1 || port > 65535)
            return false;
        conf.setIP(out[0]);
        conf.setPort(port);
    }
    else
    {
        port = std::strtol(dirs[1].c_str(), &endptr, 10);
        if (*endptr != 0 || port < 1 || port > 65535)
            return false;
        conf.setPort(port);
    }
    return true;
}

bool    Config::error_page(std::vector<std::string> &dirs, ServerConf &conf)
{
    long    code;
    char    *endptr;

    if (dirs.size() != 3)
        return false;
    code = std::strtol(dirs[1].c_str(), &endptr, 10);
    if (*endptr != 0 || code < 400 || code > 527)
        return false;
    conf.addErrorPage(static_cast<unsigned int>(code), dirs[2]);
    return true;
}

bool    Config::client_max_body_size(std::vector<std::string> &dirs, ServerConf &conf)
{
    unsigned long   size;
    char    *endptr;

    if (dirs.size() != 2)
        return false;
    size = std::strtoul(dirs[1].c_str(), &endptr, 10);
    if (*endptr != 0)
        return false;
    conf.setBodySizeLimit(size);
    return false;
}

bool    Config::server_root(std::vector<std::string> &dirs, ServerConf &conf)
{
    if (dirs.size() != 2)
        return false;
    conf.setRoot(dirs[1]);
    return true;
}

bool    Config::server_autoindex(std::vector<std::string> &dirs, ServerConf &conf)
{
    if (dirs.size() != 2)
        return false;
    if (dirs[1] == "on")
        conf.setDirListing(true);
    else if (dirs[1] == "off")
        conf.setDirListing(false);
    else
        return false;
    return true;
}

bool    Config::server_index(std::vector<std::string> &dirs, ServerConf &conf)
{
    if (dirs.size() != 2)
        return false;
    conf.setRoot(dirs[1]);
    return true;
}

bool    Config::root(std::vector<std::string> &dirs, Route &conf)
{
    if (dirs.size() != 2)
        return false;
    conf.setRoot(dirs[1]);
    return true;
}
bool    Config::rewrite(std::vector<std::string> &dirs, Route &conf)
{
    if (dirs.size() != 4)
        return false;
    if (dirs[3] == "permanent")
        conf.setRedirection(dirs[1], dirs[2], 301);
    else if (dirs[3] == "redirect")
        conf.setRedirection(dirs[1], dirs[2], 302);
    else
        return false;
    return true;
}

static void rejoin(std::vector<std::string> &dirs, std::string &out)
{
    for (size_t i = 1; i < dirs.size(); i++)
        out += dirs[i];
}

bool    Config::allowed_methods(std::vector<std::string> &dirs, Route &conf)
{
    std::string out;
    std::vector<std::string> methods;
    std::map<std::string, long>::iterator ite;
    long    perms = 0;

    if (dirs.size() <= 1)
        return false;
    rejoin(dirs, out);
    split(out, methods, ',');
    for (size_t i = 0; i < methods.size(); i++)
    {
        ite = str2permmap.find(methods[i]);
        if (ite == str2permmap.end())
            return false;
        perms |= ite->second;
    }
    conf.setMethodPerms(perms);
    return true;
}
bool    Config::autoindex(std::vector<std::string> &dirs, Route &conf)
{
    if (dirs.size() != 2)
        return false;
    if (dirs[1] == "on")
        conf.setListDirectory(true);
    else if (dirs[1] == "off")
        conf.setListDirectory(false);
    else
        return false;
    return true;
}
bool    Config::dir_default(std::vector<std::string> &dirs, Route &conf)
{
    if (dirs.size() != 2)
        return false;
    conf.setDirFile(dirs[1]);
    return true;
}
bool    Config::cgi_extension(std::vector<std::string> &dirs, Route &conf)
{
    if (dirs.size() != 2)
        return false;
    conf.setCgiExtension(dirs[1]);
    return true;
}

bool    Config::save_path(std::vector<std::string> &dirs, Route &conf)
{
    if (dirs.size() != 2)
        return false;
    conf.setUpload(true);
    conf.setSavePath(dirs[1]);
    return true;
}

const ServerConf    &Config::getServerFromHostAndIP(std::string host, std::string ip)
{
    bool        foundIP;
    size_t      server_idx = 0;

    foundIP = false;
    for (size_t i = 0; i < _servers.size(); i++)
    {
        if (_servers[i].getIP() == ip || ip == "0.0.0.0")
        {
            if (foundIP == false)
            {
                foundIP = true;
                server_idx = i;
            }
            if (_servers[i].hasServername(host))
                return _servers[i];
        }
    }
    if (foundIP == false && host != "")
        throw std::runtime_error("IP not found.");
    return _servers[server_idx];
}

const std::vector<ServerConf>   &Config::getServers()
{
    return _servers;
}
