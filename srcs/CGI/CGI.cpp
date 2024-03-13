#include "CGI.hpp"

void    CGI::getPathInfo()
{
    std::string rel_path(_request->getURN().substr(_route->getRoute().length()));
    _env["PATH_INFO"] = "";
    _env["PATH_INFO"] += _route->getRoot();
    _env["PATH_INFO"] += rel_path;
}

void    CGI::getQueryString()
{
    _env["QUERY_STRING"] = "";
    for (std::map<std::string, std::string>::const_iterator ite = _request->getURLParams().begin();
        ite != _request->getURLParams().end();
        ite++)
    {
        _env["QUERY_STRING"] += ite->first;
        _env["QUERY_STRING"] += '=';
        _env["QUERY_STRING"] += ite->second;
        if (++ite != _request->getURLParams().end())
            _env["QUERY_STRING"] += '&';
        ite--;
    }
}

void    CGI::getContentLength()
{
    std::map<std::string, std::string>::const_iterator ite = _request->getHeaders().find("Content-length");

    _env["CONTENT_LENGTH"] = "";
    if (ite == _request->getHeaders().end())
    {
        std::stringstream ss;

        ss << _request->getBody().length();
        _env["CONTENT_LENGTH"]  = ss.str();
        return ;
    }
    _env["CONTENT_LENGTH"] = ite->second;
}

void    CGI::getRequestMethod()
{
    _env["REQUEST_METHOD"]  = Config::perm2str(_request->getMethod());
}

void    CGI::getContentType()
{
    std::map<std::string, std::string>::const_iterator ite = _request->getHeaders().find("Content-type");

    _env["CONTENT_TYPE"] = "";
    if (ite == _request->getHeaders().end())
        return ;
    _env["CONTENT_TYPE"] = ite->second;
}

void    CGI::getServerName(const ServerConf &server)
{
    std::map<std::string, std::string>::const_iterator  header_ite  = _request->getHeaders().find("Host");
    std::vector<std::string>::const_iterator            sname_ite   = std::find(server.getNames().begin(), server.getNames().end(),  header_ite->second);

    _env["SERVER_NAME"] = "";
    if (header_ite == _request->getHeaders().end()
        || sname_ite == server.getNames().end())
        return ;
    _env["SERVER_NAME"] = header_ite->second;
}

void    CGI::setCGI(std::string cgiPath)
{
    _cgi_path = cgiPath;
}


CGI::CGI()
{
}

CGI::~CGI()
{
}

void    CGI::prepare(Request const &req,
                        Route const &route,
                        ServerConf const &server,
                        std::string remoteaddr)
{
    std::string query_string;
    std::string path_info;
    std::stringstream ss;

    _request    = &req;
    _route      = &route;
    getPathInfo();
    getQueryString();
    getContentLength();
    getRequestMethod();
    getContentType();
    getServerName(server);
    _env["REDIRECT_STATUS"] = "true";
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env["SCRIPT_FILENAME"] = _env["PATH_INFO"];
    _env["REMOTE_ADDR"] = remoteaddr;
    _env["REMOTE_HOST"] = "";
    _env["REMOTE_IDENT"] = "";
    _env["REMOTE_USER"] = "";
    ss << server.getPort();
    _env["SERVER_PORT"] = ss.str();
    ss.clear();
    _env["SERVER_PROTOCOL"] = "HTTP/1.1";
    _env["SERVER_SOFTWARE"] = "webserv/1.0";
}

static char *concatcpy(std::string key, std::string value)
{
    char    *ret;
    size_t  i = 0;
    size_t  j = 0;

    ret = new char[key.length() + value.length() + 2];
    for (j = 0; j < key.length(); j++)
    {
        ret[i] = key[j];
        i++;
    }
    ret[i] = '=';
    i++;
    for (j = 0; j < value.length(); j++, i++)
        ret[i] = value[j];
    ret[i] = 0;
    return ret;
}

char    **CGI::buildEnvFromAttr()
{
    char    **ret;
    size_t  i = 0;

    ret = new char*[_env.size()+1];
    for (std::map<std::string, std::string>::const_iterator ite = _env.begin();
        ite != _env.end();
        ite++)
    {
        ret[i] = concatcpy(ite->first, ite->second);
        i++;
    }
    ret[i] = NULL;
    return ret;
}

static void deleteEnv(char **env, size_t len)
{
    for (size_t i = 0; i < len; i++)
        delete env[i];
    delete[] env;
}

std::string  CGI::forwardReq()
{
    std::string result;
    char        *av[] = {NULL, NULL};
    char        **env;
    int         fds[2];
    pid_t       pid;

    if (pipe(fds) < 0)
        throw std::runtime_error("Could not open pipe.");
    pid = fork();
    if (pid == -1)
        throw std::runtime_error("Could not fork.");
    if (pid == 0)
    {
        dup2(fds[1], STDOUT_FILENO);
        dup2(fds[0], STDIN_FILENO);
        close(fds[0]);
        close(fds[1]);
        env = buildEnvFromAttr();
        execve(_cgi_path.c_str(), av, env);
        deleteEnv(env, 9);
        exit(1);
    }
    else
    {
        int     s;
        char    c;
        ssize_t rd;

        if (_env["REQUEST_METHOD"] == "POST")
            write(fds[1], _request->getBody().c_str(), _request->getBody().length());
        close(fds[1]);
        if (waitpid(pid, &s, WUNTRACED) < 0 || s != 0)
            throw std::runtime_error("Serverside error");
        do{
            rd = read(fds[0], &c, 1);
            if (rd < 0)
                throw std::runtime_error("Error while reading output of CGI.");
            result += c;
        } while (rd > 0);
        close(fds[0]);
    }
    _env.clear();
    return result;
}