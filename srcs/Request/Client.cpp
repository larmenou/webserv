#include "Client.hpp"

Client::Client(int client_fd,
                const Config *conf,
                std::string ip)
                :   _client_fd(client_fd),
                    _state(Header),
                    _type(Error),
                    _start(time(0)),
                    _conf(conf),
                    _ip(ip)
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

    return *this;
}

Client::~Client()
{
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
        _state = RespondingHeader;
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

static size_t fileSize(std::string filename)
{
	struct stat	s;

	if (stat(filename.c_str(), &s) == -1)
		return 0;
	return s.st_size;
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
	filename += _req.getURN().substr(_route.getRoute().length());
	if (isDir(filename))
		filename += "/" + _route.getDirFile();
	return (filename);
}

void    Client::bodyPostGet(char const *chunk, size_t start)
{
    std::string         filename(buildFilename());
    std::stringstream   http;
    std::string         bytes;

    _status = 200;
    try
    {
        if (!_out.is_open())
        {
            if (isDir(filename) && _route.isListingDirs())
            {
                _body_response = DirLister().generate_body(filename, _req);
                _body_len = _body_response.length();
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
        _bodyc += std::string(chunk + start).size();
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
    _body_response  = HTTPError::buildErrorPage(_server, _status);
    _state          = RespondingHeader;
}

void    Client::bodyCgi(char const *chunk, size_t start)
{
    (void) chunk; (void) start;
    std::stringstream   http;

    try {
        if (!_cgi.isStarted())
        {
            _cgi.setCGI(_route.getCgiPath());
            _cgi.prepare(_req, _route, _server, "127.0.0.1");
            _cgi.start();
        }
        if (_cgi.receive(chunk, start))
            _state = RespondingHeader;
    } catch (std::exception &e) {
        _status = std::strtol(e.what(), NULL, 10);
        _type = Error;
        _state = RespondingHeader;
        return ;
    }
}

void    Client::bodyRewrite(char const *chunk, size_t start)
{
    (void) chunk; (void) start;

    _state = RespondingHeader;
}

void    Client::bodyDelete(char const *chunk, size_t start)
{
    (void) chunk; (void) start;

    /*TODO : Implement delete method*/
    _state = RespondingHeader;
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
        ssize_t         write_size;

        if (!buildUploadPath(_req, _route, upload_path))
            throw std::runtime_error("404");
        if (!_in.is_open())
        {
            _bodyc = 0;
            _in.open(upload_path.c_str());
            if (_in.fail() || !_in.is_open())
            {
                if (fileExists(upload_path) || isDir(upload_path))
                    throw std::runtime_error("403");
                else
                    throw std::runtime_error("500");
            }
        }
        if (start != 0)
            bytes.erase(bytes.begin(), bytes.begin() + start);
        write_size = bytes.size();
        _in.write(bytes.c_str(), write_size);
        _bodyc += write_size;
        if (_bodyc < 0)
            throw std::runtime_error("500");
        if (_bodyc >= _req.getContentLength())
            _state = RespondingHeader;
    } catch (std::exception &e)
    {
        _type = Error;
        _status = std::strtol(e.what(), NULL, 10);
        _state = RespondingHeader;
        return ;
    }
}

void    Client::sendHeader()
{
    _bodyc += send(_client_fd,
                    _headers.c_str() + _bodyc, 
                    _headers.size() - _bodyc,
                    0);
    if (_bodyc >= (ssize_t)_headers.size())
    {
        _bodyc = 0;
        _state = RespondingBody;
    }
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
                        0);
    }
    else
    {
        _bodyc += write(_client_fd, 
                        _body_response.c_str() + _bodyc, 
                        _body_response.size() - _bodyc);
    }
    if (_bodyc >= (ssize_t)_body_len
        || _out.fail() || _out.eof())
    {
        _out.close();
        _bodyc = 0;
        _state = Closed;
    }
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

    http << "HTTP/1.1" << " " << _status << " " << HTTPError::getErrorString(_status) << "\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
    buildHeaderConnection(http);
    sendResponse();
}

void    Client::responseCgi()
{
    std::stringstream http;

    if (_state == RespondingHeader)
    {
        if (_headers.size() == 0)
        {
            _headers = _cgi.buildRawHeader();
            _body_response = _cgi.respond();
            _status = _cgi.getStatus();
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
                    0);
        if (_bodyc >= (ssize_t)_body_response.size())
        {
            _bodyc = 0;
            _state = Closed;
            reset();
        }
    }
}

void    Client::responseRewrite()
{
    std::stringstream http;
    
    _status = _route.getRedirCode();
    http << "HTTP/1.1" << " " << _status << " " << HTTPError::getErrorString(_status) << "\r\nLocation: " << _route.getRewrite().second << "\r\nContent-Length: 0\r\n";
    _headers = http.str();
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
    if ((size_t)_bodyc >= _server.getBodySizeLimit())
    {
        _type = Error;
        _status = 413;
    }
    ((this)->*(_body_functions[_type]))(chunk, start);
}

void    Client::respond()
{
    if ((size_t)_bodyc >= _server.getBodySizeLimit())
    {
        _type = Error;
        _status = 413;
    }
    ((this)->*(_reponse_functions[_type]))();
}

void    Client::receive()
{
    ssize_t     ret;
    size_t      body_start = 0;
    char        chunk[BUFFER_SIZE];

    ret = read(_client_fd, chunk, BUFFER_SIZE - 1);
    if (ret <= 0)
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
}

void    Client::sendResponse()
{
    send(_client_fd, _headers.c_str(), _headers.size(), 0);
    send(_client_fd, _body_response.c_str(), _body_response.size(), 0);
    _state = Closed;
    std::cout << "------ ["<< time(0) <<"] Server Response sent to client ------\n\n";
}

void    Client::reset()
{
    _body_response.clear();
    _headers.clear();
    _req.reset();
    _cgi.closeCGI();
    _bodyc = 0;
    _state = Header;
    _type = Error;
    _start = time(0);
}

bool    Client::isExpired()
{
    if (difftime(time(0), _start) > REQ_TIMEOUT)
        return true;
    if (_state == Closed)
        return true;
    return false;
}

t_clientstate   Client::getState()
{
    return _state;
}

int             Client::getFD()
{
    return _client_fd;
}