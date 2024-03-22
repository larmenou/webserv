/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rralambo <rralambo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:25 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/21 15:52:29 by rralambo         ###   ########.fr       */
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
#include "HTTPError.hpp"

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
		const std::string			_default_root;

		int startServer(int i);
		void closeServer();
		void acceptConnection(int &new_socket, int i);
		void buildResponse(Request req, int i, int client_fd);
		void sendResponse(int client_fd);
		void initPollfds(std::vector<pollfd> *pollfds);
		void addPollfd(std::vector<pollfd> *pollfds, int client_fd, int i);
		void recvDataAndBuildResp(int client_fd, int i);
		int buildCgiResp(std::string *headers, Request req, Route route, int i);
		void buildRedirResp(Route route, int client_fd);
		int buildBodyResp(std::string filename, Request req, Route route, int i);
		std::string buildFilename(Route route, Request req, int i);
		void buildHeaderConnection(std::string headers, Request req, std::stringstream *http);
		int	handleUpload(const Request &req, const Route &route, int i);
		void basicUpload(const Request &req, const Route &route);
		static void signalHandler(int);
	
	public:
		Server(std::vector<ServerConf> servers);
		~Server();

		void loop();
};
