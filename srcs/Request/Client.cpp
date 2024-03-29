#include "Client.hpp"

Client::Client(int client_fd,
                const Config *conf,
                std::string ip,
                sockaddr_in c_addr,
                sockaddr_in serv_addr)
                :   _client_fd(client_fd),
                    _client_addr(c_addr),
                    _server_addr(serv_addr),
                    _state(Header),
                    _type(Error),
                    _start(time(0)),
                    _conf(conf),
                    _ip(ip),
                    _body_response(""),
                    _headers(""),
                    _pkt_length(0),
                    _bodyc(0)
{
    _body_functions[Cgi] = &Client::bodyCgi;
    _body_functions[Put] = &Client::bodyPut;
    _body_functions[Post] = &Client::bodyPostGet;
    _body_functions[Rewrite] = &Client::bodyRewrite;
    _body_functions[Get] = &Client::bodyPostGet;
    _body_functions[Delete] = &Client::bodyDelete;
    _body_functions[Error] = &Client::bodyError;

    _reponse_functions[Cgi] = &Client::responseCgi;
    _reponse_functions[Put] = &Client::responsePut;
    _reponse_functions[Post] = &Client::responsePostGet;
    _reponse_functions[Rewrite] = &Client::responseRewrite;
    _reponse_functions[Get] = &Client::responsePostGet;
    _reponse_functions[Delete] = &Client::responseDelete;
    _reponse_functions[Error] = &Client::responseError;
}

Client::Client(Client const &a)
{
    *this = a;
}

Client  &Client::operator=(Client const &client)
{
    _client_fd = client._client_fd;
    _client_addr = client._client_addr;
    _server_addr = client._server_addr;
    _keep_alive = client._keep_alive;
    _state = client._state;
    _type = client._type;
    _status = client._status;
    _start = client._start;
    _server = client._server;
    _route = client._route;
    _conf = client._conf;
    _ip = client._ip;
    _body_functions = client._body_functions;
    _reponse_functions = client._reponse_functions;
    _body_response = client._body_response;
    _bodyc = client._bodyc;
    _headers = client._headers;

    return *this;
}

Client::~Client()
{
}

void    Client::initServerRoute()
{
    size_t  end_hostname = _req.findHeader("Host").find(":");
    std::string hostname(_req.findHeader("Host").substr(0, end_hostname));
    
    _server = _conf->getServerFromHostAndIPPort(hostname, _ip, _server_addr);
    _route = _server.findRouteFromURN(_req.getPath());
}

static bool	isDir(std::string path)
{
	struct stat	s;

	if (stat(path.c_str(), &s) == -1)
		return false;
	return (s.st_mode & S_IFMT) == S_IFDIR;
}


static bool	fileExists(std::string path)
{
	struct stat	s;

	if (stat(path.c_str(), &s) == -1)
		return false;
	return true;
}

static size_t fileSize(std::string filename)
{
	struct stat	s;

	if (stat(filename.c_str(), &s) == -1)
		return 0;
	return s.st_size;
}

void    Client::buildHeaderConnection(std::stringstream &http)
{
    std::string connection;

    if (_req.isKeepAlive())
        connection = "keep-alive";
    else
        connection = "close";
    http << "Connection: "<< connection << "\r\n" << _headers << "\r\n";
    _headers = http.str();
}

bool    Client::isCGI()
{
    std::string filename;
    std::string root;
    size_t      id;

    if (_route.getCgiPath().size() == 0)
        return false;
    if (_route.getDirFile().size() == 0)
    {
        id = _req.getPath().find(_route.getCgiExtension());
        if (id == std::string::npos)
            return false;
        if (id != _req.getPath().size() - _route.getCgiExtension().size())
            return false;
    }
    else
    {
        if (_route.getRoot().length() == 0)
            root = _server.getRoot();
        else
            root = _route.getRoot();
        filename = root + "/";
        filename += _req.getPath().substr(_route.getRoute().length());
        if (isDir(filename) && _req.getPath()[_req.getPath().size() - 1] != '/')
            return false;
        id = _route.getDirFile().find(_route.getCgiExtension());
        if (id == std::string::npos)
            return false;
        if (id != _route.getDirFile().size() - _route.getCgiExtension().size())
            return false;
    }
    return true;
}

void    Client::determineRequestType()
{
    if ((_req.getMethod() & _route.getMethodPerms()) == 0 && _req.getMethod() != DELETE)
    {
        _type = Error;
        _status = 405;
        _state = RespondingHeader;
    }
    else if (isCGI())
        _type = Cgi;
    else if (_req.getMethod() == PUT && _route.isAcceptingUploads())
        _type = Put;
    else if (_route.getRewrite().size() > 0)
        _type = Rewrite;
    else if (_req.getMethod() == POST)
        _type = Post;
    else if (_req.getMethod() == GET)
        _type = Get;
    else if (_req.getMethod() == DELETE)
        _type = Delete;
    else
    {
        _type = Error;
        _status = 501;
    }
}


std::string Client::buildFilename()
{
	std::string filename;
	std::string root;

	if (_route.getRoot().length() == 0)
		root = _server.getRoot();
	else
		root = _route.getRoot();

	filename = root + "/";
	filename += _req.getPath().substr(_route.getRoute().length());
	if (isDir(filename) && _req.getPath()[_req.getPath().size() - 1] == '/')
		filename += _route.getDirFile();
	return (filename);
}

void    Client::bodyPostGet(char const *chunk, size_t start)
{
    (void) chunk;
    std::string         filename(buildFilename());
    std::stringstream   http;
    std::string         bytes;

    _status = 200;
    try
    {
        if (!_out.is_open())
        {
            if (isDir(filename))
            {
                if (_req.getPath()[_req.getPath().size() - 1] != '/')
                {
                    std::stringstream http;

                    http << "HTTP/1.1" << " " << 302 << " " << HTTPError::getErrorString(302) 
                    << "\r\nLocation: " << _req.getPath() << '/'
                    << "\r\nContent-Length: 0\r\n\r\n";
                    _headers = http.str();
                    _state = RespondingHeader;
                    _body_len = 0;
                }
                else if (_route.isListingDirs())
                {
                    _body_response = DirLister().generate_body(filename, _req);
                    _body_len = _body_response.length();
                }
                else
                    throw std::runtime_error("404");
            }
            else if (fileExists(filename))
            {
                _out.open(filename.c_str());
                if (!_out.is_open() || _out.fail())
                    throw std::runtime_error("403");
                _body_len = fileSize(filename);
            }
            else
                throw std::runtime_error("404");
        }
        _bodyc += _pkt_length - start;
    }
    catch(const std::exception& e)
    {
        _type   = Error;
        _status = std::strtol(e.what(), NULL, 10);
        _state  = RespondingHeader;
        return ;
    }
    _state = RespondingHeader;
}

void    Client::bodyError(char const *chunk, size_t start)
{
    (void) chunk; (void) start;

    _state = RespondingHeader;
}

static std::string  remote_addr(sockaddr_in addr)
{
    std::stringstream ss;

    ss << (addr.sin_addr.s_addr & 0xff) << "."
    << ((addr.sin_addr.s_addr & 0xff00)>>8) << "."
    << ((addr.sin_addr.s_addr & 0xff0000)>>16) << "."
    << ((addr.sin_addr.s_addr & 0xff000000)>>24);

    return ss.str();
}

void    Client::bodyCgi(char const *chunk, size_t start)
{
    (void) chunk; (void) start;
    std::stringstream   http;

    try {
        if (!_cgi.isStarted())
        {
            _cgi.setCGI(_route.getCgiPath());
            _cgi.prepare(_req, _route, _server, remote_addr(_client_addr), buildFilename());
            _cgi.start();
        }
        if (_cgi.receive(chunk, start, _pkt_length))
            _state = RespondingHeader;
    } catch (std::exception &e) {
        _status = std::strtol(e.what(), NULL, 10);
        if (_status == 666)
            throw std::runtime_error("Failed to execute CGI.");
        _cgi.closeCGI();
        _type = Error;
        _state = RespondingHeader;
        return ;
    }
}

void    Client::bodyRewrite(char const *chunk, size_t start)
{
    (void) chunk; (void) start;
	std::stringstream http;

	http << "HTTP/1.1" << " " << _route.getRedirCode() << " " << HTTPError::getErrorString(_route.getRedirCode()) 
    << "\r\nLocation: " << _route.getRewrite() 
    << "\r\nContent-Length: 0\r\n\r\n";
	_headers = http.str();
    _state = RespondingHeader;
}

void    Client::bodyDelete(char const *chunk, size_t start)
{
    (void) chunk; (void) start;
    std::string filename;

    filename = buildFilename();
    if (!fileExists(filename))
    {
        _status = 404;
        _type = Error;
    }
    else
    {
        _status = 204;
        if (isDir(filename) || remove(filename.c_str()))
        {
            _status = 403;
            _type = Error;
        }
    }
    _state = RespondingHeader;
}

static bool	buildUploadPath(const Request &req, const Route &route, std::string &out)
{
    out = route.getSavePath();
    out += req.getPath().substr(route.getRoute().length());
    if (out.length() == route.getRoute().length())
        return false;
    return true;
}

void    Client::bodyPut(char const *chunk, size_t start)
{
    (void) chunk; (void) start;
    _status = 201;

    try {
        std::string     upload_path;

        if (!buildUploadPath(_req, _route, upload_path))
            throw std::runtime_error("404");
        if (!_in.is_open())
        {
            _bodyc = 0;
            if (!isDir(_route.getSavePath()))
                throw std::runtime_error("500");
            _in.open(upload_path.c_str());
            if (_in.fail() || !_in.is_open())
            {
                if (fileExists(upload_path) || isDir(upload_path))
                    throw std::runtime_error("403");
                else
                    throw std::runtime_error("500");
            }
        }
        _in.write(chunk + start, _pkt_length - start);
        _bodyc += _pkt_length;
        if (_bodyc < 0)
            throw std::runtime_error("500");
        if (_bodyc >= _req.getContentLength())
        {
            _bodyc = 0;
            _state = RespondingHeader;
        }
    } catch (std::exception &e)
    {
        _type = Error;
        _status = std::strtol(e.what(), NULL, 10);
        _bodyc = 0;
        _state = RespondingHeader;
        return ;
    }
}

void    Client::sendHeader()
{
    ssize_t ret;

    ret = send(_client_fd,
                    _headers.c_str() + _bodyc, 
                    _headers.size() - _bodyc,
                    MSG_NOSIGNAL);
    if (ret < 0)
    {
        _keep_alive = false;
        resetOrClose();
    }
    _bodyc += ret;
    if (_bodyc >= (ssize_t)_headers.size())
    {
        _bodyc = 0;
        _state = RespondingBody;
    }
}

void    Client::sendBodyResponse()
{
    ssize_t ret;

    ret = send(_client_fd,
                _body_response.c_str() + _bodyc, 
                _body_response.size() - _bodyc,
                MSG_NOSIGNAL);
    if (ret < 0)
    {
        _keep_alive = false;
        resetOrClose();
    }
    if (_bodyc >= (ssize_t)_body_response.size())
        resetOrClose();
}

void    Client::sendFile()
{
    char buff[BUFFER_SIZE];

    if (_body_response.size() == 0)
    {
        _out.seekg(_bodyc);
        _out.read(buff, BUFFER_SIZE);
        _bodyc += send(_client_fd,
                        buff, 
                        _out.gcount(),
                        MSG_NOSIGNAL);
    }
    else
    {
        _bodyc += send(_client_fd, 
                        _body_response.c_str() + _bodyc, 
                        _body_response.size() - _bodyc,
                        MSG_NOSIGNAL);
    }
    if (_bodyc >= (ssize_t)_body_len
        || _out.fail() || _out.eof())
        resetOrClose();
}

void    Client::responsePostGet()
{
    std::stringstream   http;

    if (_state == RespondingHeader)
    {
        if (_headers.size() == 0)
        {
            http << "HTTP/1.1" << " " << _status << " " << HTTPError::getErrorString(_status) << "\r\nContent-Type: text/html\r\nContent-Length: " << _body_len << "\r\n";
            buildHeaderConnection(http);
            _bodyc = 0;
        }
        sendHeader();
    }
    if (_state == RespondingBody)
        sendFile();
}

void    Client::responsePut()
{
    std::stringstream http;

    if (_headers.size() == 0)
    {
        http << "HTTP/1.1" << " " << _status << " " << HTTPError::getErrorString(_status) << "\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
        buildHeaderConnection(http);
        _bodyc = 0;
    }
    if (_state == RespondingHeader)
        sendHeader();
    if (_state == RespondingBody)
        resetOrClose();
}

void    Client::responseCgi()
{
    std::stringstream http;

    if (_state == RespondingHeader)
    {
        if (_headers.size() == 0)
        {
            try
            {
                _body_response = _cgi.respond();
                _headers = _cgi.buildRawHeader();
                _status = _cgi.getStatus();
                if (_status >= 400)
                    _body_response = HTTPError::buildErrorPage(_server, _status);
            }
            catch(const std::exception& e)
            {
                _cgi.closeCGI();
                _body_response = HTTPError::buildErrorPage(_server,
                                    _status = std::strtol(e.what(), NULL, 10));
            }
            _body_len = _body_response.size();
            http << "HTTP/1.1" << " " << _status << " " << HTTPError::getErrorString(_status) << "\r\nContent-Length: " << _body_len << "\r\n";
            buildHeaderConnection(http);
            _bodyc = 0;
        }
        sendHeader();
    }
    if (_state == RespondingBody)
    {
        _bodyc += send(_client_fd,
                    _body_response.c_str() + _bodyc, 
                    _body_response.size() - _bodyc,
                    MSG_NOSIGNAL);
        if (_bodyc >= (ssize_t)_body_response.size())
            resetOrClose();
    }
}

void    Client::responseRewrite()
{
    if (_state == RespondingHeader)
        sendHeader();
    if (_state == RespondingBody)
        resetOrClose();
}

void    Client::responseDelete()
{
    std::stringstream http;

    if (_headers.size() == 0)
    {
        http << "HTTP/1.1" << " " << _status << " " << HTTPError::getErrorString(_status) << "\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
        buildHeaderConnection(http);
    }
    if (_state == RespondingHeader)
        sendHeader();
    if (_state == RespondingBody)
        resetOrClose();
}

void    Client::responseError()
{
    std::stringstream http;

    if (_headers.size() == 0)
    {
        _body_response  = HTTPError::buildErrorPage(_server, _status);
        http << "HTTP/1.1" << " " << _status << " " << HTTPError::getErrorString(_status) << "\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
        buildHeaderConnection(http);
    }
    if (_state == RespondingHeader)
        sendHeader();
    if (_state == RespondingBody)
        sendBodyResponse();
}

void    Client::processBody(char const *chunk, size_t start)
{
    if ((size_t)_bodyc + _pkt_length >= _server.getBodySizeLimit())
    {
        _bodyc = 0;
        _type = Error;
        _state = RespondingHeader;
        _status = 413;
    }
    ((this)->*(_body_functions[_type]))(chunk, start);
}

void    Client::respond()
{
    if (_state == RespondingHeader || _state == RespondingBody)
    {
        _start = time(0);
        ((this)->*(_reponse_functions[_type]))();
    }
}

void    Client::receive(const char *chunk, size_t pkt_len)
{
    size_t      body_start = 0;

    _start = time(0);
    _pkt_length = pkt_len;
    if (_state == Header)
    {
        try
        {
            body_start = _req.receive_header(chunk, _pkt_length);
        } catch (std::exception &e)
        {
            _status = std::strtol(e.what(), NULL, 10);
            _type = Error;
            _state = RespondingHeader;
        }
        if (_req.isParsed())
        {
            std::cerr << _req << std::endl;
            initServerRoute();
            determineRequestType();
            _state = Body;
            _bodyc = 0;
        }
    }
    if (_state == Body)
        processBody(chunk, body_start);
}

void    Client::resetOrClose()
{
    if (_req.isKeepAlive())
    {
        _body_response.clear();
        _headers.clear();
        _req.reset();
        _cgi.closeCGI();
        _out.close();
        _in.close();
        _bodyc = 0;
        _state = Header;
        _type = Error;
        _start = time(0);
    }
    else
        _state = Closed;
}

bool    Client::isExpired()
{
    if (difftime(time(0), _start) > REQ_TIMEOUT)
        return true;
    if (_state == Closed)
        return true;
    return false;
}

void    Client::close()
{
    _state = Closed;
}

t_clientstate   Client::getState()
{
    return _state;
}

int             Client::getFD()
{
    return _client_fd;
}