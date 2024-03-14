/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:29 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/14 08:37:12 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "RegisteredUsers.hpp"

Server::Server(std::string ip_address, int port) : _ip_address(ip_address), _port(port), _socket_listen(), _fds(), _socketAddress(), _socketAddress_len(sizeof(_socketAddress)), _body_response()
{
	_socketAddress.sin_family = AF_INET;
	_socketAddress.sin_port = htons(_port);
	_socketAddress.sin_addr.s_addr = INADDR_ANY;

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
	
	if (req.getURN() == "/" || req.getURN() == "/index.html")
	{
		_body_response = "<!DOCTYPE html><html lang=\"en\"><head><title>WebServer</title></head><body><h1> HOME </h1><p> Hello from your Server :) </p><a href=\"test.html\">Test</a><p></p><a href=\"form.html\"> Connexion</a></body></html>";
		http << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
	}
	else if (req.getURN() == "/test.html")
	{
		_body_response = "<!DOCTYPE html><html lang=\"en\"><head><title>WebServer</title></head><body><h1> TEST </h1><p> Teeeeeest </p><a href=\"index.html\"> Index</a><p></p><a href=\"truc.html\">Truc</a> : vers 404</body></html>";
		http << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
	}
	else if (req.getURN() == "/form.html")
	{
		_body_response = "<!DOCTYPE html><html><head><title>Register</title></head><body><h1>Register</h1><form action=\"/index.html\" method=\"post\"><label for=\"nom\">Nom : </label><input type=\"text\" id=\"nom\" name=\"nom\" required><br><br><label for=\"password\">Mot de passe : </label><input type=\"password\" id=\"password\" name=\"password\" required><br><input type=\"submit\" value=\"Envoyer\"></form></body></html>";
		http << "HTTP/1.1 201 Created\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
	}
	else if (req.getURN() == "/favicon.ico")
	{}
	else
	{
		_body_response = "<!DOCTYPE html><html lang=\"en\"><head><title>WebServer</title></head><body><h1> ERROR 404 </h1></body></html>";
		http << "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " << _body_response.length() << "\r\n";
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