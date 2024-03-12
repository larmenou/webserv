/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:29 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/12 14:34:18 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Request.hpp"

Server::Server(std::string ip_address, int port) : _ip_address(ip_address), _port(port), _socket_listen(), _fds(), _socketAddress(), _socketAddress_len(sizeof(_socketAddress)), _body_response()
{
	//std::stringstream ss;
	_socketAddress.sin_family = AF_INET;
	_socketAddress.sin_port = htons(_port);
	_socketAddress.sin_addr.s_addr = INADDR_ANY;

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
	std::stringstream http;
	
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
				bytesReceived = read(_fds[i].fd, buffer, BUFFER_SIZE - 1);
				if (bytesReceived > 0)
				{
					buffer[bytesReceived] = '\0';
					std::cout << "------ Received Request from client ------\n\n";
					//std::cout << buffer << std::endl;

					std::stringstream ss(buffer);
					std::string str = ss.str();
					Request req(str);
					std::cout << req;

					/* _body_response = "<!DOCTYPE html><html lang=\"en\"><head><title>WebServer</title></head><body><h1> HOME </h1><p> Hello from your Server :) </p><a href=\"test.html\">Test</a></body></html>";
					http << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n\r\n" << _body_response;
					_header_response = http.str();
					write(_fds[i].fd, _header_response.c_str(), _header_response.length()); */

					if (req.getURN() == "/" || req.getURN() == "/index.html")
					{
						_body_response = "<!DOCTYPE html><html lang=\"en\"><head><title>WebServer</title></head><body><h1> HOME </h1><p> Hello from your Server :) </p><a href=\"test.html\">Test</a></body></html>";
						http << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n\r\n";
						_header_response = http.str();
						write(_fds[i].fd, _header_response.c_str(), _header_response.size());
						write(_fds[i].fd, _body_response.c_str(), _body_response.size());
					}
					else if (req.getURN() == "/test.html")
					{
						_body_response = "<!DOCTYPE html><html lang=\"en\"><head><title>WebServer</title></head><body><h1> TEST </h1><p> Teeeeeest </p><a href=\"index.html\"> Index</a><a href=\"truc.html\"> Truc : vers 404</a></body></html>";
						http << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n\r\n";
						_header_response = http.str();
						write(_fds[i].fd, _header_response.c_str(), _header_response.size());
						write(_fds[i].fd, _body_response.c_str(), _body_response.size());
					}
					else if (req.getURN() == "/favicon.ico")
					{}
					else
					{
						_body_response = "<!DOCTYPE html><html lang=\"en\"><head><title>WebServer</title></head><body><h1> ERROR 404 </h1></body></html>";
						http << "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n\r\n";
						_header_response = http.str();
						write(_fds[i].fd, _header_response.c_str(), _header_response.size());
						write(_fds[i].fd, _body_response.c_str(), _body_response.size());
					}
					
					//sendResponse(i);
					_header_response = "";
					_body_response = "";
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
	write(_fds[i].fd, _header_response.c_str(), _header_response.size());
	//send(_fds[i].fd, _body_response.c_str(), _body_response.size(), 0);
	std::cout << "------ Server Response sent to client ------\n\n";
}