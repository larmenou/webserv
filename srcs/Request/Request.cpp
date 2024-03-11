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
        }
        if (strbuff == CRLF)
            break;
    }
    return true;
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
        throw std::runtime_error("Invalid request.");
    method = Config::str2perm(method_str);
    if (method == -1)
        throw std::runtime_error("Unsupported method.");
}

void    Request::parseLineHeader(std::string &line)
{
    std::vector<std::string> out;
    split(line, out, ':');
    if (out.size() != 2)
        return ;
    _headers[out[0]] = out[1];
}

void    Request::parseFromRaw(std::string &raw)
{
    std::stringstream   ss(raw);
    std::string         line;
    getURNFromSS(ss, _method, _urn, _http_ver);
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
    os << "Body:" << req.getBody() << std::endl;
    return os;
}