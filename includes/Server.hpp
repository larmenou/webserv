/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:25 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/28 10:52:50 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <deque>
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
#include "HTTPError.hpp"
#include "Client.hpp"

#ifndef BUFF_SIZE
# define BUFF_SIZE 30720
#endif

#ifndef MAX_CLIENTS
# define MAX_CLIENTS 200
#endif

class Server
{
	private:
		Config						&_conf;
		std::vector<ServerConf>		_servers;
		std::vector<int>			_sockets_listen;
		std::deque<pollfd>			_clients_fds;
		std::deque<Client>			_clients;
		std::vector<sockaddr_in>	_socketAddresses;
		std::string 				_header_response;
		std::string 				_body_response;
		const std::string			_default_root;

		int 			startServer(int i);
		void 			closeServer();
		sockaddr_in 	acceptConnection(int &new_socket, int i);
		void 			initPollfds(std::vector<pollfd> *pollfds);
		void 			addPollfd(std::vector<pollfd> *pollfds, int client_fd, sockaddr_in client_addr, int i);

		std::string 	parseReferer(std::string referer);
		static void 	signalHandler(int);

	public:
		Server(Config &servers);
		~Server();

		void loop();
};
