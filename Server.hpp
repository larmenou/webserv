/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:25 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/06 14:58:23 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>

#define BUFFER_SIZE 30720

class Server
{
	private:
		std::string			_ip_address;
		int					_port;
		int					_socket;
		int					_new_socket;
		struct sockaddr_in	_socketAddress;
        unsigned int		_socketAddress_len;
		std::string 		_serverMessage;

		int startServer();
		void closeServer();
		void acceptConnection(int &new_socket);
		std::string buildResponse(std::string filename);
		void sendResponse();
	
	public:
		Server(std::string ip_address, int port);
		~Server();

		void startListen();
};