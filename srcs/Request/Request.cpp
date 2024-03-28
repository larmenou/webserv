#include "Request.hpp"


bool    getlineCRLF(std::stringstream &ss, std::string &str)
{
    std::string CRLF("\r\n");
    size_t  i = 0;
    char c;
    std::string strbuff;

    str.clear();
    if (ss.eof())
        return false;
    while(ss.get(c))
    {
        if (c == CRLF[i])
        {
            strbuff += c;
            i++;
        }
        else
        {
            str += c;
            str += strbuff;
            i = 0;
            strbuff.clear();
        }
        if (strbuff == CRLF)
            break;
    }
    return true;
}

void    trimstr(std::string &str)
{
    std::string tmp;

    size_t  begin = str.find_first_not_of(" \t\n\v\f\r");
    if (begin == std::string::npos)
        return ;
    size_t  end = str.find_last_not_of(" \t\n\v\f\r");

    tmp = str.substr(begin, end - begin + 1);
    str = tmp;
}

void strtolower(std::string &str)
{
    for (size_t i = 0; i < str.length(); i++)
        str[i] = std::tolower(str[i]);
}

Request::Request() : _content_length(0), _isParsed(false), _keep_alive(true)
{
}

Request::~Request()
{
}

bool                Request::isParsed() const { return _isParsed; }
bool                Request::isKeepAlive() const { return _keep_alive; }
ssize_t             Request::getContentLength() const { return _content_length; }
const std::string   Request::getPath() const { return _path; }
const std::string   Request::getHTTPVersion() const { return _http_ver; }
const std::string   Request::getBody() const { return _body; }
long                Request::getMethod() const { return _method; }
const std::map<std::string, std::string>  &Request::getHeaders() const { return _headers; }
const std::string  &Request::getURLParams() const { return _query_string; }

const std::string   Request::findHeader(std::string key) const
{
    strtolower(key);
    std::map<std::string, std::string>::const_iterator ite = _headers.find(key);
    if (ite == _headers.end())
        return "";
    return ite->second;
}

bool    Request::checkExtension(std::string extension) const
{
    size_t  id = _path.find(extension);
    if (id != std::string::npos && id == _path.size() - extension.size())
        return true;
    return false;
}

void    Request::parseLineHeader(std::string &line)
{
    size_t  sep_i;
    std::string value;
    std::string key;

    sep_i = line.find(":");
    if (sep_i == std::string::npos)
        throw std::runtime_error("400");
    key = line.substr(0, sep_i);
    value = line.substr(sep_i + 1, std::string::npos);
    trimstr(key); trimstr(value);
    strtolower(key);
    _headers[key] = value;
}

void    Request::extractGETParams()
{
    size_t  s = _path.find("?");
    if (s == std::string::npos || s + 1 > _path.size())
        return ;
    size_t  e = _path.find("#", s);
    _query_string = _path.substr(s + 1, e - s);

    _path = _path.substr(0, s);
}

void    Request::validatePath()
{
    size_t  f = 0;

    if (_path.size() > MAX_PATH)
        throw std::runtime_error("414");
    if (_path[0] != '/')
        throw std::runtime_error("400");
    while ((f = _path.find("../")) != std::string::npos)
    {
        _path[f] = '/';
        _path[f+1] = '/';
    }
}

static void getPathFromSS(std::stringstream &ss,
                    long        &method,
                    std::string &path, 
                    std::string &http_ver)
{
    std::string line;
    std::string method_str;
    getlineCRLF(ss, line);
    std::stringstream l(line);

    if (!((l >> method_str) && (l >> path) && (l >> http_ver)))
        throw std::runtime_error("400");
    method = Config::str2perm(method_str);
    if (method == -1)
        throw std::runtime_error("501");
}

void    Request::parseFromRaw(std::string raw)
{
    std::stringstream   ss(raw);
    std::string         line;

    getPathFromSS(ss, _method, _path, _http_ver);
    extractGETParams();
    validatePath();
    while (getlineCRLF(ss, line))
    {
        if (line.length() == 0)
            break ;
        parseLineHeader(line);
    }
}

size_t  Request::receive_header(const char *chunk, size_t pkt_length)
{

    size_t              start = _raw_header.length();
    size_t              end;

    _raw_header += std::string(chunk, pkt_length);
    end = _raw_header.find("\r\n\r\n");
    if (end != std::string::npos)
    {
        parseFromRaw(_raw_header);
        std::string         clen(findHeader("content-length"));
        std::string         conheader(findHeader("connection"));
        if (conheader == "close")
            _keep_alive = false;
        if (clen != "")
        {
            trimstr(clen);
            _content_length = std::strtoul(clen.c_str(), NULL, 10);
        }
        _isParsed = true;
        return end + 4;
    }
    else if (_raw_header.length() > MAX_HEADER)
        throw std::runtime_error("431");
    return end - start;
}

void    Request::reset()
{
    _raw_header.clear();
    _path.clear();
    _http_ver.clear();
    _body.clear();
    _headers.clear();
    _isParsed = false;
    _keep_alive = true;
    _method = 0;
}

std::ostream    &operator<<(std::ostream &os, const Request &req)
{
    os << "[" << time(0) << "] " << Config::perm2str(req.getMethod());
    os << " " << req.getPath();
    os << " " << req.getHTTPVersion();
    os << " User-Agent : \"";
    os << req.findHeader("user-agent");
    os << "\" " << req.getURLParams();
    
    return os;
}