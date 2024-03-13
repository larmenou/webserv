/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:25 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/13 10:46:54 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>

#include "Request.hpp"

#define BUFF_SIZE 30720
#define MAX_EVENTS 20

class Server
{
	private:
		std::string			_ip_address;
		int					_port;
		int					_socket_listen;
		struct pollfd 		_fds[MAX_EVENTS];
		struct sockaddr_in	_socketAddress;
        unsigned int		_socketAddress_len;
		std::string 		_header_response;
		std::string 		_body_response;

		int startServer();
		void closeServer();
		void acceptConnection(int &new_socket);
		void buildResponse(Request req);
		void sendResponse(int i);
	
	public:
		Server(std::string ip_address, int port);
		~Server();

		void startListen();
};