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

Request::Request(std::string &raw_req)
{
    parseFromRaw(raw_req);
}

Request::~Request()
{
}

const std::string   Request::getURN() const { return _urn; }
const std::string   Request::getHTTPVersion() const { return _http_ver; }
const std::string   Request::getBody() const { return _body; }
long                Request::getMethod() const { return _method; }
const std::map<std::string, std::string>  &Request::getHeaders() const { return _headers; }
const std::map<std::string, std::string>  &Request::getURLParams() const { return _getParams; }

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
    size_t  id = _urn.find(extension);
    if (id != std::string::npos && id == _urn.size() - extension.size())
        return true;
    return false;
}



static void getURNFromSS(std::stringstream &ss,
                    long        &method,
                    std::string &urn, 
                    std::string &http_ver)
{
    std::string line;
    std::string method_str;
    getlineCRLF(ss, line);
    std::stringstream l(line);
    if (!((l >> method_str) && (l >> urn) && (l >> http_ver)))
        throw std::runtime_error("400");
    method = Config::str2perm(method_str);
    if (method == -1)
        throw std::runtime_error("501");
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
    size_t  s = _urn.find("?");
    if (s == std::string::npos || s + 1 > _urn.size())
        return ;
    std::string params(_urn.substr(s + 1));
    std::vector<std::string>    out;
    split(params, out, '&');
    for (size_t i = 0; i < out.size(); i++)
    {
        if (out[i].length() == 0)
            continue;
        size_t eq = out[i].find('=');
        _getParams[out[i].substr(0, eq)] = eq != std::string::npos ? out[i].substr(eq + 1, out[i].size() - eq) : "";
    }
    _urn = _urn.substr(0, s);
}

void    Request::parseFromRaw(std::string &raw)
{
    std::stringstream   ss(raw);
    std::string         line;
    getURNFromSS(ss, _method, _urn, _http_ver);
    extractGETParams();
    while (getlineCRLF(ss, line))
    {
        if (line.length() == 0)
            break;
        parseLineHeader(line);
    }
    if (ss.tellg() != -1 && _method & POST)
        _body = ss.str().substr(ss.tellg());
}

std::ostream    &operator<<(std::ostream &os, const Request &req)
{
    std::map<std::string, std::string>::const_iterator ite = req.getHeaders().begin();

    os << "Method:" << req.getMethod() << std::endl;
    os << "URN:" << req.getURN() << std::endl;
    os << "HTTP Version:" << req.getHTTPVersion() << std::endl;
    os << "Headers:" << std::endl;
    for (; ite != req.getHeaders().end(); ite++)
        os << "\t" << ite->first << ":" << ite->second << std::endl;
    os << "URLParams:" << std::endl;
    ite = req.getURLParams().begin();
    for (; ite != req.getURLParams().end(); ite++)
        os << "\t" << ite->first << ":" << ite->second << std::endl;
    os << "Body:" << req.getBody() << std::endl;
    return os;
}