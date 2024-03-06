/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:29 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/06 10:33:49 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(std::string ip_address, int port) : _ip_address(ip_address), _port(port), _socket(), _new_socket(), _socketAddress(), _socketAddress_len(sizeof(_socketAddress)), _serverMessage(buildResponse())
{
	_socketAddress.sin_family = AF_INET;
	_socketAddress.sin_port = htons(_port);
	_socketAddress.sin_addr.s_addr = inet_addr(_ip_address.c_str());

	if (startServer() != 0)
	{
		std::cout << "Failed to start server with PORT: " << ntohs(_socketAddress.sin_port);
		exit(1);
	}
}

Server::~Server()
{
	closeServer();
}

int Server::startServer()
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0)
	{
		std::cout << "Cannot create socket" << std::endl;
		return (1);
	}
	if (bind(_socket, (sockaddr *)&_socketAddress, _socketAddress_len) < 0)
	{
		std::cout << "Cannot connect socket to address" << std::endl;
		return (1);
	}
	
	return (0);
}

void Server::closeServer()
{
	close(_socket);
	close(_new_socket);
	exit(0);
}

void Server::startListen()
{
	int bytesReceived;
	
	if (listen(_socket, 20) < 0)
	{
		std::cout << "Socket listen failed" << std::endl;
		exit(1);
	}

	std::cout << "\n*** Listening on ADDRESS: " << inet_ntoa(_socketAddress.sin_addr) << " PORT: " << ntohs(_socketAddress.sin_port) << " ***\n\n";

	while (true)
	{
		std::cout << "====== Waiting for a new connection ======\n\n\n";
		acceptConnection(_new_socket);

		char buffer[BUFFER_SIZE] = {0};
		bytesReceived = read(_new_socket, buffer, BUFFER_SIZE);
		
		if (bytesReceived < 0)
		{
			std::cout << "Failed to read bytes from client socket connection" << std::endl;
			exit(1);
		}

		std::cout << "------ Received Request from client ------\n\n";
		std::cout << buffer << std::endl;

		sendResponse();

		close(_new_socket);
	}
}

void Server::acceptConnection(int &new_socket)
{
	new_socket = accept(_socket, (sockaddr *)&_socketAddress, &_socketAddress_len);
	if (new_socket < 0)
	{
		std::cout << "Server failed to accept incoming connection from ADDRESS: " << inet_ntoa(_socketAddress.sin_addr) << "; PORT: " << ntohs(_socketAddress.sin_port) << std::endl;
		exit(1);
	}
}

std::string Server::buildResponse()
{
	std::ostringstream ss;
	std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>";
	
	ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
		<< htmlFile;

	return ss.str();
}

void Server::sendResponse()
{
	unsigned long bytesSent;

	bytesSent = write(_new_socket, _serverMessage.c_str(), _serverMessage.size());

	if (bytesSent == _serverMessage.size())
	{
		std::cout << "------ Server Response sent to client ------\n\n";
	}
	else
	{
		std::cout << "Error sending response to client" << std::endl;
	}
}