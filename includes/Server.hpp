/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:25 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/15 09:53:21 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#include "Request.hpp"

#define BUFF_SIZE 30720
#define MAX_EVENTS 20

class Server
{
	private:
		std::string					_ip_address;
		int							_port;
		int							_socket_listen;
		struct pollfd 				_fds[MAX_EVENTS];
		struct sockaddr_in			_socketAddress;
        unsigned int				_socketAddress_len;
		std::string 				_header_response;
		std::string 				_body_response;
		std::map<int, std::string>	_status_code;
		std::string					_default_dir;
		Route						_route;
		std::vector<ServerConf>		_servers;

		int startServer();
		void closeServer();
		void acceptConnection(int &new_socket);
		void buildResponse(Request req);
		void sendResponse(int i);
	
	public:
		Server(std::string ip_address, int port, Route route, std::vector<ServerConf> servers);
		~Server();

		//void setDefaultDir(std::string dir);

		void startListen();
};