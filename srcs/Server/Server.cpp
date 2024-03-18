/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:29 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/18 14:14:24 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "RegisteredUsers.hpp"

Server::Server(std::vector<ServerConf> servers) : _servers(servers), _sockets_listen(), _socketAddresses(), _header_response(), _body_response()
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
			}
			bytesReceived = read(client_fd, buffer, BUFF_SIZE - 1);
			if (bytesReceived > 0)
			{
				buffer[bytesReceived] = '\0';
				std::cout << "------ Received Request from client ------\n\n";

				std::cout << buffer << std::endl;

				std::stringstream ss(buffer);
				std::string str = ss.str();
				Request req(str);

				//std::cout << req << std::endl;
				
				if (req.getBody().length() != 0)
				{
					std::string str = req.getBody();
					users.addDb(str);
				}

				buildResponse(req, i, client_fd);
			}
			close(client_fd);
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

void Server::buildResponse(Request req, int i, int client_fd)
{
	std::stringstream http;
	std::string filename;
	int fd;
	int status = 200;
	const Route &route = _servers[i].findRouteFromURN(req.getURN());

	_body_response.clear();

	if (req.getURN() != "/favicon.ico")
	{
		if (req.getURN() == "/redirect")
		{
			status = 301;
			std::cout << route.getRewrite().second << std::endl;
			http << "HTTP/1.1" << " " << status << " " << _status_code[status] << "\r\nLocation: " << route.getRewrite().second << "\r\nContent-Length: 0\r\n";
			_header_response = http.str();
			
			send(client_fd, _header_response.c_str(), _header_response.size(), 0);
			return ;
		}
		else
		{
			filename = route.getRoot();
			filename += req.getURN();
			
			if (filename == route.getRoot() + "/")
				filename += route.getDirFile();

			fd = open(filename.c_str(), O_RDONLY);
			if (fd == -1)
			{
				if (_servers[i].getErrorPage(404).size() > 0)
				{
					fd = open("./html/404error.html", O_RDONLY);
					status = 404;
				}
			}

			char c;
			while (read(fd, &c, 1) > 0)
				_body_response += c;
			
			http << "HTTP/1.1" << " " << status << " " << _status_code[status] << "\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
		
			close(fd);
		}
	}
	
	if (req.getHeaders().find("Connection")->second == " keep-alive")
	{
		http << "Connection: keep-alive\r\n\r\n";
		_header_response = http.str();
	}
	else
	{
		http << "Connection: close\r\n\r\n";
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