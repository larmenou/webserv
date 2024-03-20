/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:25 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/20 13:57:23 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <csignal>
#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#include "Request.hpp"
#include "DirLister.hpp"

#define BUFF_SIZE 30720
#define MAX_EVENTS 20

class Server
{
	private:
		std::vector<ServerConf>		_servers;
		std::vector<int>			_sockets_listen;
		std::vector<sockaddr_in>	_socketAddresses;
		std::string 				_header_response;
		std::string 				_body_response;
		std::map<int, std::string>	_status_code;
		const std::string			_default_root;

		int startServer(int i);
		void closeServer();
		void acceptConnection(int &new_socket, int i);
		void buildResponse(Request req, int i, int client_fd);
		void sendResponse(int client_fd);

		static void signalHandler(int);
	
	public:
		Server(std::vector<ServerConf> servers);
		~Server();

		void loop();
};
