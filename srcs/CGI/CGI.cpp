#include "CGI.hpp"

void    CGI::getQueryString()
{
    _env["QUERY_STRING"] = _request->getURLParams();
}

void    CGI::getContentLength()
{
    std::stringstream ss("");

    ss << _request->getContentLength();
    _env["CONTENT_LENGTH"] = ss.str();
}

void    CGI::getRequestMethod()
{
    _env["REQUEST_METHOD"]  = Config::perm2str(_request->getMethod());
}

void    CGI::getContentType()
{
    _env["CONTENT_TYPE"] = _request->findHeader("Content-type");
}

void    CGI::getServerName()
{
    std::string hostname(_request->findHeader("host"));
    size_t      end;

    if ((end = hostname.find(':')) != std::string::npos)
        _env["SERVER_NAME"] = hostname.substr(0, end);
    else
        _env["SERVER_NAME"] = hostname;
}

static std::string  addPrefixCapitalize(std::string key)
{
    std::string str("HTTP_");

    for (size_t i = 0; i < key.size(); i++)
    {
        if (std::isalpha(key[i]))
            str += std::toupper(key[i]);
        else
            str += "_";
    }
    return str;
}

void    CGI::getHeaders()
{
    for (std::map<std::string, std::string>::const_iterator ite = _request->getHeaders().begin();
        ite != _request->getHeaders().end();
        ite++)
        _env[addPrefixCapitalize(ite->first)] = ite->second;
}


void    CGI::setCGI(std::string cgiPath)
{
    _cgi_path = cgiPath;
}


CGI::CGI() : _env_execve(NULL),
                _pid(-1),
                _is_started(false),
                _bdc(0)
{
    _fds_in[0] = -1;
    _fds_in[1] = -1;
    _fds_out[0] = -1;
    _fds_out[1] = -1;

}

void    CGI::freeExecEnv()
{
    if (_env_execve != NULL)
    {
        for (size_t i = 0; i < _env.size(); i++)
            delete[] _env_execve[i];
        delete[] _env_execve;
    }
    _env_execve = NULL;
}

CGI::~CGI()
{
    closeCGI();
}

void    CGI::prepare(Request const &req,
                        Route const &route,
                        ServerConf const &server,
                        std::string remoteaddr,
                        std::string path_info)
{
    std::stringstream ss;

    _request    = &req;
    _route      = &route;
    _status     = 200;
    _env.clear();
    getQueryString();
    getContentLength();
    getRequestMethod();
    getContentType();
    getServerName();
    getHeaders();
    _env["PATH_INFO"] = path_info;
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

void    CGI::childProc()
{
    char        *av[] = {NULL, NULL};

    dup2(_fds_out[1], STDOUT_FILENO);
    close(_fds_out[0]);
    dup2(_fds_in[0], STDIN_FILENO);
    close(_fds_in[1]);
    try {
        _env_execve = buildEnvFromAttr();
    }
    catch(const std::exception& e) {
        throw std::runtime_error("666");
    }
    close(_fds_in[0]);
    close(_fds_out[1]);
    execve(_cgi_path.c_str(), av, _env_execve);
    close(_fds_out[1]);
    close(_fds_in[0]);
    throw std::runtime_error("666");
}

void    CGI::parentProc()
{
    _raw_response.clear();
    close(_fds_in[0]);
    close(_fds_out[1]);
}

void    CGI::start()
{
    if (_env.size() == 0)
        throw std::runtime_error("500");
    if (pipe(_fds_in) < 0)
        throw std::runtime_error("500");
    if (pipe(_fds_out) < 0)
        throw std::runtime_error("500");
    if (fcntl(_fds_out[0], F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("500");
    _pid = fork();
    if (_pid == -1)
        throw std::runtime_error("500");
    if (_pid == 0)
        childProc();
    else
        parentProc();
    _headers.clear();
    _env.clear();
    _is_started = true;
}

void    CGI::closeCGI()
{
    freeExecEnv();
    if (_fds_in[0] != -1)
        close(_fds_in[0]);
    if (_fds_in[1] != -1)
        close(_fds_in[1]);
    if (_fds_out[0] != -1)
        close(_fds_out[0]);
    if (_fds_out[1] != -1)
        close(_fds_out[1]);
    if (_pid > 0)
        kill(_pid, SIGKILL);
    _env.clear();
    _headers.clear();
    _body.clear();
    _cgi_path.clear();
    _raw_response.clear();
    _is_started = false;
}

bool    CGI::receive(const char *chunk, size_t start, size_t _pkt_len)
{
    ssize_t rd;

    if (_request->getMethod() & POST)
    {
        rd = write(_fds_in[1], chunk + start, _pkt_len - start);
        if (rd < 0)
            throw std::runtime_error("500");
        _bdc += rd;
        if (_bdc >= _request->getContentLength())
            return close(_fds_in[1]), true;
    } else
        return close(_fds_in[1]), true;
    return false;
}

void    CGI::parseHeader(std::stringstream &ss)
{
    std::string line;
    std::string value;
    std::string key;
    size_t  sep_i;

    while (getlineCRLF(ss, line))
    {
        if (line.length() == 0)
            break;
        sep_i = line.find(":");
        if (sep_i == std::string::npos)
            continue;
        key = line.substr(0, sep_i);
        value = line.substr(sep_i + 1, std::string::npos);
        trimstr(key);
        _headers[key] = value;
    }
}

void    CGI::readAllWait()
{
    ssize_t rd;
    clock_t start;
    int     s;
    int     ret;
    char    c;
    bool    stop = true;

    start = clock();
    while (stop)
    {
        rd = read(_fds_out[0], &c, 1);
        switch (rd)
        {
            case -1:
                if (errno == EAGAIN)
                {
                    if ((clock() - start) / CLOCKS_PER_SEC >= GATEWAY_TIMEOUT)
                        throw std::runtime_error("504");
                    break;
                }
                else
                    throw std::runtime_error("500");
            case 0:
                stop = false;
            default:
                _raw_response += c;
        }
    }
    start = clock();
    while ((ret = waitpid(_pid, &s, WUNTRACED)) == 0)
    {
        if (((clock() - start) / CLOCKS_PER_SEC) >= GATEWAY_TIMEOUT )
        {
            kill(_pid, SIGKILL);
            throw std::runtime_error("504");
        }
    }
    if (ret < 0)
        throw std::runtime_error("500");
    if (WIFEXITED(s) && WEXITSTATUS(s) == 127)
        throw std::runtime_error("503");
}

std::string CGI::respond()
{
    std::vector<std::string>    out;
    std::stringstream           ss;
    std::string                 line;

    readAllWait();
    if (_raw_response[0] == 0)
        throw std::runtime_error("500");
    ss << _raw_response;
    _body.clear();
    _headers.clear();
    parseHeader(ss);
    if (ss.tellg() != -1)
        _body = ss.str().substr(ss.tellg());
    std::map<std::string, std::string>::const_iterator ite = _headers.find("Status");
    if (ite != _headers.end())
    {
        _status = std::strtol(ite->second.c_str(), NULL, 10);
        _headers.erase(ite->first);
    }
    return _body;
}

bool    CGI::isStarted() const {
    return _is_started;
}

int     CGI::getStatus() const
{
    return _status;
}

const std::string   &CGI::getRawResp() const
{
    return _raw_response;
}

const std::string   &CGI::getBody() const
{
    return _body;
}

std::string         CGI::buildRawHeader() const
{
    std::map<std::string, std::string>::const_iterator ite = _headers.begin();
    std::string ret;

    for (;ite != _headers.end(); ite++)
    {
        ret += ite->first;
        ret += ": ";
        ret += ite->second;
        ret += "\r\n";
    }
    return ret;
}


const std::map<std::string, std::string> &CGI::getHeaders() const
{
    return _headers;
}
