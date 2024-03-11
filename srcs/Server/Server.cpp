/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:29 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/11 14:28:11 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(std::string ip_address, int port) : _ip_address(ip_address), _port(port), _socket_listen(), _fds(), _socketAddress(), _socketAddress_len(sizeof(_socketAddress)), _serverMessage(buildResponse("index.html"))
{
	//std::stringstream ss;
	_socketAddress.sin_family = AF_INET;
	_socketAddress.sin_port = htons(_port);
	_socketAddress.sin_addr.s_addr = inet_addr(_ip_address.c_str());

	/* ss << port;
	std::string port_str = ss.str();
	getaddrinfo("WebServ", port_str.c_str(), NULL, NULL); */
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
	char buffer[BUFFER_SIZE];
	
	if (listen(_socket_listen, 20) < 0)
	{
		std::cout << "Socket listen failed" << std::endl;
		exit(1);
	}

	std::cout << "\n*** Listening on ADDRESS: " << inet_ntoa(_socketAddress.sin_addr) << " PORT: " << ntohs(_socketAddress.sin_port) << " ***\n\n";

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
				bytesReceived = read(_fds[i].fd, buffer, BUFFER_SIZE - 1);
				if (bytesReceived > 0)
				{
					buffer[bytesReceived] = '\0';
					std::cout << "------ Received Request from client ------\n\n";
					std::cout << buffer << std::endl;

					_serverMessage = "HTTP/1.1 200 OK\r\nContent-Length: 11\r\n\r\nHello World";
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
		std::cout << "Server failed to accept incoming connection from ADDRESS: " << inet_ntoa(_socketAddress.sin_addr) << "; PORT: " << ntohs(_socketAddress.sin_port) << std::endl;
		exit(1);
	}
}

std::string Server::buildResponse(std::string filename)
{
	std::ostringstream ss;
	std::string htmlFile/*  = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>" */;
	std::fstream fs;
	std::stringstream buffer;
	
	fs.open(filename.c_str());
	buffer << fs.rdbuf();
	htmlFile = buffer.str();
	if (filename == "404error.html")
	{
		ss << "HTTP/1.1 404 Not Found\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
			<< htmlFile;
	}
	else
	{
		ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
			<< htmlFile;
	}
	fs.close();

	return ss.str();
}

void Server::sendResponse(int i)
{
	unsigned long bytesSent;

	bytesSent = write(_fds[i].fd, _serverMessage.c_str(), _serverMessage.size());

	if (bytesSent == _serverMessage.size())
	{
		std::cout << "------ Server Response sent to client ------\n\n";
	}
	else
	{
		std::cout << "Error sending response to client" << std::endl;
	}
}