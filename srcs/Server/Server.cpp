/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:29 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/27 09:18:31 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "CGI.hpp"

int g_sig;

void Server::signalHandler(int)
{
	g_sig = 1;
}

Server::Server(Config &conf) : 	_conf(conf),
								_servers(conf.getServers()),
								_sockets_listen(), 
								_socketAddresses(), 
								_header_response(), 
								_body_response(), 
								_default_root("./html")
{
	struct sockaddr_in socketAddress;
	g_sig = 0;
	
	signal(SIGINT, &Server::signalHandler);
	signal(SIGQUIT, &Server::signalHandler);

	for (unsigned int i = 0; i < _conf.getServers().size(); i++)
	{
		socketAddress.sin_family = AF_INET;
		socketAddress.sin_port = htons(_conf.getServers()[i].getPort());
		socketAddress.sin_addr.s_addr = INADDR_ANY;
		_socketAddresses.push_back(socketAddress);

		if (startServer(i) != 0)
		{
			std::cerr << "Failed to start server with PORT: " << ntohs(_socketAddresses[i].sin_port) << std::endl;
			exit(1);
		}
	}
}

Server::~Server()
{
	closeServer();
	signal(SIGINT, SIG_DFL);
}

int Server::startServer(int i)
{
	int socket_listen = socket(AF_INET, SOCK_STREAM, 0);
	int enable = 1;
	
	if (setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		std::cerr << "Error setting SO_REUSEADDR" << std::endl;
		return (1);
	}
	if (socket_listen < 0)
	{
		std::cerr << "Cannot create socket" << std::endl;
		return (1);
	}
	if (bind(socket_listen, (sockaddr *)&_socketAddresses[i], sizeof(_socketAddresses[i])) < 0)
	{
		std::cerr << "Cannot connect socket to address" << std::endl;
		return (1);
	}
	if (listen(socket_listen, 20) < 0)
	{
		std::cerr << "Socket listen failed" << std::endl;
		exit(1);
	}

	std::cerr << "\n*** Listening on ADDRESS: " << _conf.getServers()[i].getIP() << " PORT: " << _conf.getServers()[i].getPort() << " ***\n\n";
	
	_sockets_listen.push_back(socket_listen);
	return (0);
}

void Server::closeServer()
{
	for (unsigned int i = 0; i < _sockets_listen.size(); i++)
		close(_sockets_listen[i]);
}

void Server::initPollfds(std::vector<pollfd> *pollfds)
{
	for (unsigned int i = 0; i < _sockets_listen.size(); i++)
	{
        pollfd pfd;
        pfd.fd = _sockets_listen[i];
        pfd.events = POLLIN;
        pollfds->push_back(pfd);
    }
}

void Server::addPollfd(std::vector<pollfd> *pollfds, int client_fd, sockaddr_in client_addr, int i)
{
	pollfd new_pollfd = {0, 0, 0};

	new_pollfd.fd = client_fd;
	new_pollfd.events = POLLIN | POLLOUT;
	pollfds->push_back(new_pollfd);
	_clients.push_back(Client(client_fd, &_conf, _servers[i].getIP(), client_addr));
	std::cerr << "[" << client_fd << "] New connection (CONN_COUNT="<<  _clients.size() << ")" << std::endl;
}

std::string Server::parseReferer(std::string referer)
{
	std::vector<std::string> out;

	split(referer, out, '/');
	if (out[out.size() - 1].find("html") != std::string::npos)
		return (out[out.size() - 1]);
	return ("");
}

void Server::loop()
{
	int 				ready;
	int 				client_fd;
	std::vector<pollfd> pollfds;
	sockaddr_in			client_addr;
	size_t				pkt_len;
	char        		chunk[BUFFER_SIZE];
	
	initPollfds(&pollfds);
	while (true)
	{
		if (g_sig)
		{
			close(client_fd);
			break ;
		}
		ready = poll(pollfds.data(), pollfds.size(), 100);
		if (ready == -1)
		{
			if (errno == EINTR)
			{
				close(client_fd);
				break ;
			}
			std::cerr << "Poll failed." << std::endl;
			break ;
		}

		if (_clients.size() < MAX_CLIENTS)
		{
			for (unsigned int i = 0; i < _servers.size(); i++)
			{
				if (pollfds[i].revents & POLLIN)
				{
					client_addr = acceptConnection(client_fd, i);
					if (client_fd != -1)
						addPollfd(&pollfds, client_fd, client_addr, i);
				}
			}
		}
		for (size_t i = _servers.size(); i < pollfds.size(); i++)
		{
			size_t j = i - _servers.size();

			if (pollfds[i].revents & POLLIN &&
			(_clients[j].getState() == Header || _clients[j].getState() == Body ))
			{
				pkt_len = recv(pollfds[i].fd, chunk, BUFFER_SIZE - 1, MSG_NOSIGNAL);
				if (pkt_len > 0)
					_clients[j].receive(chunk, pkt_len);
				else
					_clients[j].close();
			}
			else if ((pollfds[i].revents & POLLOUT) &&
			(_clients[j].getState() == RespondingHeader || _clients[j].getState() == RespondingBody))
				_clients[j].respond();
			if (_clients[j].isExpired())
			{
				std::cerr << "[" << pollfds[i].fd << "] Closed connection.(CONN_COUNT=" <<  _clients.size() << ")" << std::endl;
				close(pollfds[i].fd);
				pollfds.erase(pollfds.begin() + i);
				_clients.erase(_clients.begin() + j);
			}
		}
	}
}

sockaddr_in	Server::acceptConnection(int &new_socket, int i)
{
	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	new_socket = accept(_sockets_listen[i], (sockaddr *)&client_addr, &client_len);
	if (new_socket == -1)
		std::cerr << "Server failed to accept incoming connection" << std::endl;
	return client_addr;
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

int Server::buildCgiResp(std::string *headers, Request const &req, Route route, int i)
{
	CGI cgi;
	int status;

	cgi.setCGI("/usr/bin/php-cgi");
	cgi.prepare(req,route,_servers[i],"127.0.0.1", buildFilename(route, req, i));
	try {
		cgi.start();
		_body_response = cgi.getBody();
		status = cgi.getStatus();
		*headers = cgi.buildRawHeader();
	} catch (std::exception &e) {
		_body_response = HTTPError::buildErrorPage(_servers[i], 
													status = std::strtol(e.what(), NULL, 10));
	}
	return (status);
}

void Server::buildRedirResp(Route route, int client_fd)
{
	int status;
	std::stringstream http;
	
	status = route.getRedirCode();
	http << "HTTP/1.1" << " " << status << " " << HTTPError::getErrorString(status) << "\r\nLocation: " << route.getRewrite() << "\r\nContent-Length: 0\r\n";
	_header_response = http.str();
	
	send(client_fd, _header_response.c_str(), _header_response.size(), 0);
}

int Server::buildBodyResp(std::string filename, Request const &req, Route route, int i)
{
	int status = 200;
	int fd;

	if (isDir(filename) && route.isListingDirs())
		_body_response = DirLister().generate_body(filename, req);
	else if (fileExists(filename))
	{
		fd = open(filename.c_str(), O_RDONLY);
		if (fd == -1)
			_body_response = HTTPError::buildErrorPage(_servers[i], status = 403);
		char c;
		while (read(fd, &c, 1) > 0)
			_body_response += c;
		close(fd);
	}
	else
		_body_response = HTTPError::buildErrorPage(_servers[i], status = 404);
	return (status);
}

std::string Server::buildFilename(Route route, Request const &req, int i)
{
	std::string filename;
	std::string root;
	
	if (route.getRoot().length() == 0)
		root = _servers[i].getRoot();
	else
		root = route.getRoot();
	filename = root;
	filename += req.getURN().substr(route.getRoute().length());
	if (isDir(filename))
		filename += "/" + route.getDirFile();
	return (filename);
}

void Server::buildHeaderConnection(std::string headers, Request const &req, std::stringstream *http)
{
	if (req.getHeaders().find("connection")->second == "keep-alive")
	{
		*http << "Connection: keep-alive\r\n" << headers << "\r\n";
		_header_response = http->str();
	}
	else
	{
		*http << "Connection: close\r\n" << headers << "\r\n";
		_header_response = http->str();
	}
}

static bool	buildUploadPath(const Request &req, const Route &route, std::string &out)
{	
	out = route.getSavePath();
	out += req.getURN().substr(route.getRoute().length());

	return true;
}

void Server::basicUpload(const Request &req, const Route &route)
{
	std::ofstream	of;
	std::string		upload_path;

	buildUploadPath(req, route, upload_path);
	of.open(upload_path.c_str(), std::ios::out);
	if (!of.is_open())
	{
		if (fileExists(upload_path) || isDir(upload_path))
			throw std::runtime_error("403");
		else
			throw std::runtime_error("500");
	}
	of << req.getBody();
	of.close();
}

int	Server::handleUpload(const Request &req, const Route &route, int i)
{
	std::string					content_type;
	int							status = 201;

	strtolower(content_type = req.findHeader("content-type"));
	try {
		basicUpload(req, route);
	} catch (std::exception &e)
	{
		_body_response = HTTPError::buildErrorPage(_servers[i], status = std::strtol(e.what(), NULL, 10));
	}
	return status;
}
