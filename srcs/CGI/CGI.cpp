#include "CGI.hpp"

void    CGI::getPathInfo()
{
    std::string rel_path(_request->getURN().substr(_route->getRoute().length()));
    _path_info += _route->getRoot();
    _path_info += rel_path;
}

void    CGI::getQueryString()
{
    for (std::map<std::string, std::string>::const_iterator ite = _request->getURLParams().begin();
        ite != _request->getURLParams().end();
        ite++)
    {
        _query_string += ite->first;
        _query_string += '=';
        _query_string += ite->second;
        if (ite++ != _request->getURLParams().end())
            _query_string += '&';
        ite--;
    }
}

void    CGI::getContentLength()
{
    std::map<std::string, std::string>::const_iterator ite = _request->getHeaders().find("Content-length");

    if (ite == _request->getHeaders().end())
    {
        std::stringstream ss;

        ss << _request->getBody().length();
        _content_length = ss.str();
        return ;
    }
    _content_length = ite->second;
}

void    CGI::getRequestMethod()
{
    _request_method = Config::perm2str(_request->getMethod());
}

void    CGI::getContentType()
{
    std::map<std::string, std::string>::const_iterator ite = _request->getHeaders().find("Content-type");

    if (ite == _request->getHeaders().end())
        return ;
    _content_type = ite->second;
}

CGI::CGI()
{
}

CGI::~CGI()
{
}

void    CGI::prepare(Request const &req, Route &route)
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

    ss << "export REDIRECT_STATUS=true \n";
    ss << "export QUERY_STRING='" << _query_string << "' \n";
    ss << "export PATH_INFO='" << _path_info << "' \n";
    ss << "export SCRIPT_FILENAME=" << _path_info << " \n";
    ss << "export REQUEST_METHOD='" << _request_method << "' \n";
    ss << "export CONTENT_LENGTH='" << _content_length << "' \n";
    ss << "export CONTENT_TYPE='" << _content_type << "' \n";
    ss << "export GATEWAY_INTERFACE=CGI/1.1 \n";
    ss << "php-cgi";
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
    char **ret;

    ret = new char*[9];
    ret[0] = concatcpy("REDIRECT_STATUS", "true");
    ret[1] = concatcpy("QUERY_STRING", _query_string);
    ret[2] = concatcpy("PATH_INFO", _path_info);
    ret[3] = concatcpy("SCRIPT_FILENAME", _path_info);
    ret[4] = concatcpy("REQUEST_METHOD", _request_method);
    ret[5] = concatcpy("CONTENT_LENGTH", _content_length);
    ret[6] = concatcpy("CONTENT_TYPE", _content_type);
    ret[7] = concatcpy("GATEWAY_INTERFACE", "CGI/1.1");
    ret[8] = NULL;
    return ret;
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
    env = buildEnvFromAttr();
    if (pid == -1)
        throw std::runtime_error("Could not fork");
    if (pid == 0)
    {
        execve("/usr/bin/php-cgi", av, env);
        exit(1);
    }
    else
    {

    }
    return result;
}