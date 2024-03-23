#include "Client.hpp"

Client::Client(int client_fd,
                const Config *conf,
                std::string ip)
                :   _client_fd(client_fd),
                    _state(Header),
                    _type(Error),
                    _start(clock()),
                    _conf(conf),
                    _ip(ip),
                    _fd(-1)
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
    _headers = client._headers;
    _fd = client._fd;

    return *this;
}

Client::~Client()
{
    if (_fd != -1)
        close(_fd);
}

void    Client::initServerRoute()
{
    size_t  end_hostname = _req.findHeader("hostname").find(":");
    std::string hostname(_req.findHeader("hostname").substr(0, end_hostname));
    
    _server = _conf->getServerFromHostAndIP(hostname, _ip);
    _route = _server.findRouteFromURN(_req.getURN());
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

void    Client::determineRequestType()
{
    if ((_req.getMethod() & _route.getMethodPerms()) == 0)
    {
        _type = Error;
        _status = 405;
        _state = Responding;
    }
    else if (_req.checkExtension(_route.getCgiExtension()))
        _type = Cgi;
    else if (_req.getMethod() == PUT && _route.isAcceptingUploads())
        _type = Put;
    else if (_route.getRewrite().second.size() > 0)
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

std::string Client::buildFilename()
{
	std::string filename;
	std::string root;
	
	if (_route.getRoot().length() == 0)
		root = _server.getRoot();
	else
		root = _route.getRoot();
	filename = root;
	filename += _req.getURN().substr(_route.getRoute().length());
	if (isDir(filename))
		filename += "/" + _route.getDirFile();
	return (filename);
}

void    Client::bodyPostGet(char const *chunk, size_t start)
{
    (void) chunk; (void) start;
    std::string         filename(buildFilename());
    std::stringstream   http;
    std::string         bytes;

    _status = 200;
    try
    {
        if (isDir(filename) && _route.isListingDirs())
            _body_response = DirLister().generate_body(filename, _req);
        else if (fileExists(filename))
        {
            _fd = open(filename.c_str(), O_RDONLY);
            if (_fd == -1)
                throw std::runtime_error("403");
        }
        else
            throw std::runtime_error("404");
        
    }
    catch(const std::exception& e)
    {
        _type = Error;
        _status = std::strtol(e.what(), NULL, 10);
        _state = Responding;
        return ;
    }
    _state = Responding;
}

void    Client::bodyError(char const *chunk, size_t start)
{
    (void) chunk; (void) start;
    _body_response = HTTPError::buildErrorPage(_server, _status);
    _state = Responding;
}

void    Client::bodyCgi(char const *chunk, size_t start)
{
    (void) chunk; (void) start;
    std::stringstream   http;

    _cgi.setCGI(_route.getCgiPath());
    _cgi.prepare(_req, _route, _server, "127.0.0.1");
    try {
        _cgi.forwardReq();
        _body_response = _cgi.getBody();
        _status = _cgi.getStatus();
        _headers = _cgi.buildRawHeader();
    } catch (std::exception &e) {
        _status = std::strtol(e.what(), NULL, 10);
        _type = Error;
        _state = Responding;
        return ;
    }
    _state = Responding;
}

void    Client::bodyRewrite(char const *chunk, size_t start)
{
    (void) chunk; (void) start;

    _state = Responding;
}

void    Client::bodyDelete(char const *chunk, size_t start)
{
    (void) chunk; (void) start;

    /*TODO : Implement delete method*/
    _state = Responding;
}

static bool	buildUploadPath(const Request &req, const Route &route, std::string &out)
{
    out = route.getSavePath();
    out += req.getURN().substr(route.getRoute().length());
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
        std::string     bytes(chunk);
        ssize_t          write_size;

        if (!buildUploadPath(_req, _route, upload_path))
            throw std::runtime_error("404");
        if (_fd < 0)
        {
            _bodyc = 0;
            _fd = open(upload_path.c_str(), O_WRONLY | O_CREAT, 0655);
            if (_fd < 0)
            {
                if (fileExists(upload_path) || isDir(upload_path))
                    throw std::runtime_error("403");
                else
                    throw std::runtime_error("500");
            }
        }
        if (_start != 0)
            bytes.erase(bytes.begin(), bytes.begin() + start);
        write_size = bytes.size();
        _bodyc += write(_fd, bytes.c_str(), write_size);
        if (_bodyc < 0)
            throw std::runtime_error("500");
        if (_bodyc >= _req.getContentLength())
            _state = Responding;
    } catch (std::exception &e)
    {
        _type = Error;
        _status = std::strtol(e.what(), NULL, 10);
        _state = Responding;
        return ;
    }
}

void    Client::responsePostGet()
{
    std::stringstream http;

    http << "HTTP/1.1" << " " << _status << " " << HTTPError::getErrorString(_status) << "\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
    buildHeaderConnection(http);
    sendResponse();
}

void    Client::responsePut()
{
    std::stringstream http;

    http << "HTTP/1.1" << " " << _status << " " << HTTPError::getErrorString(_status) << "\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
    buildHeaderConnection(http);
    sendResponse();
}

void    Client::responseCgi()
{
    std::stringstream http;

    http << "HTTP/1.1" << " " << _status << " " << HTTPError::getErrorString(_status) << "\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
    buildHeaderConnection(http);
    sendResponse();
}

void    Client::responseRewrite()
{
    std::stringstream http;
    
    _status = _route.getRedirCode();
    http << "HTTP/1.1" << " " << _status << " " << HTTPError::getErrorString(_status) << "\r\nLocation: " << _route.getRewrite().second << "\r\nContent-Length: 0\r\n";
    _headers = http.str();

    _state = Responding;
    sendResponse();
}

void    Client::responseDelete()
{
    sendResponse();
}

void    Client::responseError()
{
    std::stringstream http;

    _body_response = HTTPError::buildErrorPage(_server, _status);
    http << "HTTP/1.1" << " " << _status << " " << HTTPError::getErrorString(_status) << "\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
    buildHeaderConnection(http);
    sendResponse();
}

void    Client::processBody(char const *chunk, size_t start)
{
   ((this)->*(_body_functions[_type]))(chunk, start);
}

void    Client::respond()
{
    ((this)->*(_reponse_functions[_type]))();
}

void    Client::receive()
{
    ssize_t     ret;
    size_t      body_start = 0;
    char        chunk[BUFFER_SIZE];

    if (_state == Responding)
        return ;
    ret = read(_client_fd, chunk, BUFFER_SIZE - 1);
    if (ret < 0)
        return ;
    chunk[ret] = 0;
    if (_state == Header)
    {
        try
        {
            body_start = _req.receive_header(chunk);
        } catch (std::exception &e)
        {
            _status = std::strtol(e.what(), NULL, 10);
            _type = Error;
        }
        if (body_start != std::string::npos)
        {
            initServerRoute();
            determineRequestType();
            _state = Body;
        }
    }
    if (_state == Body)
        processBody(chunk, body_start);
    if (_state == Responding)
        respond();
}

void    Client::sendResponse()
{
    std::cout << "Sent :\n" << _headers << std::endl;
    send(_client_fd, _headers.c_str(), _headers.size(), 0);
    send(_client_fd, _body_response.c_str(), _body_response.size(), 0);
    if (_req.isKeepAlive())
        _state = Waiting;
    else
        _state = Closed;
    std::cout << "------ Server Response sent to client ------\n\n";
}

bool    Client::isExpired()
{
    if (((clock() - _start) / CLOCKS_PER_SEC) > REQ_TIMEOUT)
        return true;
    if (_state == Closed)
        return true;
    return false;
}

t_clientstate   Client::getState()
{
    return _state;
}