/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:29 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/19 13:27:12 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "RegisteredUsers.hpp"
#include "CGI.hpp"

Server::Server(std::vector<ServerConf> servers) : _servers(servers), _sockets_listen(), _socketAddresses(), _header_response(), _body_response(), _default_root("./html")
{
	struct sockaddr_in socketAddress;

	_status_code.insert(std::pair<int, std::string>(200, "OK"));
	_status_code.insert(std::pair<int, std::string>(201, "Created"));
	_status_code.insert(std::pair<int, std::string>(301, "Moved Permanently"));
	_status_code.insert(std::pair<int, std::string>(401, "Unauthorized"));
	_status_code.insert(std::pair<int, std::string>(404, "Not Found"));
	_status_code.insert(std::pair<int, std::string>(413, "Payload Too Large"));

	for (unsigned int i = 0; i < servers.size(); i++)
	{
		socketAddress.sin_family = AF_INET;
		socketAddress.sin_port = htons(_servers[i].getPort());
		socketAddress.sin_addr.s_addr = INADDR_ANY;
		_socketAddresses.push_back(socketAddress);

		if (startServer(i) != 0)
		{
			std::cout << "Failed to start server with PORT: " << ntohs(_socketAddresses[i].sin_port) << std::endl;
			exit(1);
		}
	}
}

Server::~Server()
{
	closeServer();
}

int Server::startServer(int i)
{
	int socket_listen = socket(AF_INET, SOCK_STREAM, 0);
	int enable = 1;
	
	if (setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		std::cerr << "Error setting SO_REUSEADDR" << std::endl;
	}
	if (socket_listen < 0)
	{
		std::cout << "Cannot create socket" << std::endl;
		return (1);
	}
	if (bind(socket_listen, (sockaddr *)&_socketAddresses[i], sizeof(_socketAddresses[i])) < 0)
	{
		std::cout << "Cannot connect socket to address" << std::endl;
		return (1);
	}
	if (listen(socket_listen, 20) < 0)
	{
		std::cout << "Socket listen failed" << std::endl;
		exit(1);
	}

	std::cout << "\n*** Listening on ADDRESS: " << _servers[i].getIP() << " PORT: " << _servers[i].getPort() << " ***\n\n";
	
	_sockets_listen.push_back(socket_listen);
	return (0);
}

void Server::closeServer()
{
	for (unsigned int i = 0; i < _sockets_listen.size(); i++)
		close(_sockets_listen[i]);
}

void Server::loop()
{
	int bytesReceived;
	int ready;
	int client_fd;
	char buffer[BUFF_SIZE];
	RegisteredUsers users;
	
	std::vector<pollfd> pollfds;
	
    for (unsigned int i = 0; i < _sockets_listen.size(); i++)
	{
        pollfd pfd;
        pfd.fd = _sockets_listen[i];
        pfd.events = POLLIN;
        pollfds.push_back(pfd);
    }
	
	while (true)
	{
		std::cout << "====== Waiting for a new connection ======\n\n\n";

		ready = poll(pollfds.data(), pollfds.size(), -1);
		if (ready == -1)
		{
			std::cout << "Poll failed." << std::endl;
			break ;
		}

		for (unsigned int i = 0; i < pollfds.size(); i++)
		{
			if (pollfds[i].revents & POLLIN)
			{
				acceptConnection(client_fd, i);
				if (client_fd != -1)
				{
					pollfd new_pollfd;
					new_pollfd.fd = client_fd;
					new_pollfd.events = POLLIN;
					pollfds.insert(pollfds.begin() + i + 1, new_pollfd);
					
					bytesReceived = read(client_fd, buffer, BUFF_SIZE - 1);
					if (bytesReceived > 0)
					{
						buffer[bytesReceived] = '\0';
						std::cout << "------ Received Request from client ------\n\n";

						std::cout << buffer << std::endl;

						std::stringstream ss(buffer);
						std::string str = ss.str();
						Request req(str);
						
						if (req.getBody().length() != 0)
						{
							std::string str = req.getBody();
							users.addDb(str);
						}

						buildResponse(req, i, client_fd);
					}
					close(client_fd);
					pollfds.erase(pollfds.begin() + i + 1);
				}
			}
		}
	}
}

void Server::acceptConnection(int &new_socket, int i)
{
	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	new_socket = accept(_sockets_listen[i], (sockaddr *)&client_addr, &client_len);
	if (new_socket < 0)
	{
		std::cout << "Server failed to accept incoming connection" << std::endl;
		exit(1);
	}
}

static bool	isDir(std::string path)
{
	struct stat	s;

	if (stat(path.c_str(), &s) == -1)
		return false;
	return (s.st_mode & S_IFMT) == S_IFDIR;
}

void Server::buildResponse(Request req, int i, int client_fd)
{
	std::stringstream http;
	std::string filename;
	std::string headers;
	int fd;
	int status = 200;
	const Route &route = _servers[i].findRouteFromURN(req.getURN());

	_body_response.clear();

	if (req.getURN() != "/favicon.ico")
	{
		if (route.getRoot().size() > 0)
		{
			filename = route.getRoot();
			filename += req.getURN().substr(route.getRoute().length() - 1);
			if (filename == route.getRoot() + "/")
				filename += route.getDirFile();
		}
		else if (route.getRewrite().second.size() > 0)
		{
			status = 301;
			http << "HTTP/1.1" << " " << status << " " << _status_code[status] << "\r\nLocation: " << route.getRewrite().second << "\r\nContent-Length: 0\r\n";
			_header_response = http.str();
			
			send(client_fd, _header_response.c_str(), _header_response.size(), 0);
			return ;
		}
		else
		{
			filename = _default_root;
			filename += req.getURN();
			if (filename == _default_root + "/")
				filename += "index.html";
		}

		if (req.checkExtension(route.getCgiExtension()))
		{
			CGI cgi;

			std::cout << "Send to CGI" << std::endl;
			cgi.setCGI("/usr/bin/php-cgi");
			cgi.prepare(req,route,_servers[i],"127.0.0.1");
			cgi.forwardReq();
			_body_response = cgi.getBody();
			status = cgi.getStatus();
			headers = cgi.buildRawHeader();
		}
		else
		{
			if (isDir(filename) && route.isListingDirs())
				_body_response = DirLister().generate_body(filename, req);
			else
			{
				fd = open(filename.c_str(), O_RDONLY);
				if (fd == -1)
				{
					fd = open((_servers[i].getRoot() + "/" + _servers[i].getErrorPage(404)).c_str(), O_RDONLY);
					status = 404;
				}

				char c;
				while (read(fd, &c, 1) > 0)
					_body_response += c;
				close(fd);
			}
		}
		http << "HTTP/1.1" << " " << status << " " << _status_code[status] << "\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
	}
	
	if (req.getHeaders().find("Connection")->second == " keep-alive")
	{
		http << "Connection: keep-alive\r\n" << headers << "\r\n\r\n";
		_header_response = http.str();
	}
	else
	{
		http << "Connection: close\r\n" << headers << "\r\n\r\n";
		_header_response = http.str();
	}
	
	http.clear();
	http.str("");

	sendResponse(client_fd);
}

void Server::sendResponse(int client_fd)
{
	send(client_fd, _header_response.c_str(), _header_response.size(), 0);
	send(client_fd, _body_response.c_str(), _body_response.size(), 0);
	std::cout << "------ Server Response sent to client ------\n\n";
}