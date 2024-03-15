/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:29 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/15 10:44:10 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "RegisteredUsers.hpp"

Server::Server(std::string ip_address, int port, Route route, std::vector<ServerConf> servers) : _ip_address(ip_address), _port(port), _socket_listen(), _fds(), _socketAddress(), _socketAddress_len(sizeof(_socketAddress)), _body_response()
{
	_socketAddress.sin_family = AF_INET;
	_socketAddress.sin_port = htons(_port);
	_socketAddress.sin_addr.s_addr = INADDR_ANY;

	_status_code.insert(std::pair<int, std::string>(200, "OK"));
	_status_code.insert(std::pair<int, std::string>(201, "Created"));
	_status_code.insert(std::pair<int, std::string>(401, "Unauthorized"));
	_status_code.insert(std::pair<int, std::string>(404, "Not Found"));

	_route = route;
	_servers = servers;
	//_default_dir = _servers;

	if (startServer() != 0)
	{
		std::cout << "Failed to start server with PORT: " << ntohs(_socketAddress.sin_port) << std::endl;
		exit(1);
	}
}

Server::~Server()
{
	closeServer();
}

int Server::startServer()
{
	_socket_listen = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_listen < 0)
	{
		std::cout << "Cannot create socket" << std::endl;
		return (1);
	}
	if (bind(_socket_listen, (sockaddr *)&_socketAddress, _socketAddress_len) < 0)
	{
		std::cout << "Cannot connect socket to address" << std::endl;
		return (1);
	}
	
	return (0);
}

void Server::closeServer()
{
	close(_socket_listen);
	exit(0);
}

void Server::startListen()
{
	int bytesReceived;
	int	nfds;
	int ready;
	int client_fd;
	char buffer[BUFF_SIZE];
	RegisteredUsers users;
	
	if (listen(_socket_listen, 20) < 0)
	{
		std::cout << "Socket listen failed" << std::endl;
		exit(1);
	}

	std::cout << "\n*** Listening on ADDRESS: " << _ip_address << " PORT: " << ntohs(_socketAddress.sin_port) << " ***\n\n";

	_fds[0].fd = _socket_listen;
	_fds[0].events = POLLIN;
	nfds = 1;
	
	while (true)
	{
		std::cout << "====== Waiting for a new connection ======\n\n\n";

		ready = poll(_fds, nfds, -1);
		if (ready == -1)
		{
			std::cout << "Poll failed." << std::endl;
			break ;
		}

		for (int i = 0; i < nfds; i++)
		{
			if (_fds[i].revents & POLLIN)
			{
				if (_fds[i].fd == _socket_listen)
				{
					acceptConnection(client_fd);
					_fds[nfds].fd = client_fd;
					_fds[nfds].events = POLLIN;
					nfds++;
				}
			}
			else
			{
				bytesReceived = read(_fds[i].fd, buffer, BUFF_SIZE - 1);
				if (bytesReceived > 0)
				{
					buffer[bytesReceived] = '\0';
					std::cout << "------ Received Request from client ------\n\n";

					std::cout << buffer << std::endl;

					std::stringstream ss(buffer);
					std::string str = ss.str();
					Request req(str);

					std::cout << req << std::endl;

					if (req.getBody().length() != 0)
					{
						std::string str = req.getBody();
						users.addDb(str);
					}

					buildResponse(req);
					
					sendResponse(i);
				}
				close(_fds[i].fd);
				nfds--;
				i--;
			}
		}
	}
}

void Server::acceptConnection(int &new_socket)
{
	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	new_socket = accept(_socket_listen, (sockaddr *)&client_addr, &client_len);
	if (new_socket < 0)
	{
		std::cout << "Server failed to accept incoming connection" << std::endl;
		exit(1);
	}
}

void Server::buildResponse(Request req)
{
	std::stringstream http;
	std::string filename;
	int fd;
	int status = 200;

	_body_response.clear();

	if (req.getURN() != "/favicon.ico")
	{
		filename = "./html";
		filename += req.getURN();
		
		if (filename == "./html/")
			filename += "index.html";

		fd = open(filename.c_str(), O_RDONLY);
		if (fd == -1)
		{
			fd = open("./html/404error.html", O_RDONLY);
			status = 404;
		}

		char c;
		while (read(fd, &c, 1) > 0)
			_body_response += c;
		
		http << "HTTP/1.1" << " " << status << " " << _status_code[status] << "\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
	
		close(fd);
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
}

void Server::sendResponse(int i)
{
	send(_fds[i].fd, _header_response.c_str(), _header_response.size(), 0);
	send(_fds[i].fd, _body_response.c_str(), _body_response.size(), 0);
	std::cout << "------ Server Response sent to client ------\n\n";
}