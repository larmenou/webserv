/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:42:29 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/28 13:55:47 by larmenou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "CGI.hpp"

int g_sig;

void Server::signalHandler(int)
{
	g_sig = 1;
}

Server::Server(Config &conf) : 	_conf(conf),
								_servers(conf.getServers()),
								_sockets_listen(), 
								_socketAddresses(), 
								_header_response(), 
								_body_response(), 
								_default_root("./html")
{
	struct sockaddr_in socketAddress;
	std::set<std::pair<in_addr_t, in_port_t> >::const_iterator ite;
	g_sig = 0;

	signal(SIGINT, &Server::signalHandler);
	signal(SIGQUIT, &Server::signalHandler);

	ite = _conf.getListens().begin();
	for (size_t i = 0; ite != _conf.getListens().end(); ite++, i++)
	{
		socketAddress.sin_family = AF_INET;
		socketAddress.sin_port = ite->second;
		socketAddress.sin_addr.s_addr = ite->first;
		_socketAddresses.push_back(socketAddress);

		if (startServer(i) != 0)
		{
			closeServer();
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
			std::cerr << "Failed to start server with PORT: " << ntohs(_socketAddresses[i].sin_port) << std::endl;
			throw std::runtime_error("Port error");
		}
	}
}

Server::~Server()
{
	closeServer();
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
}

static void	printStartServer(in_addr_t addr, in_port_t port)
{
	std::cerr << "\n*** Listening on ADDRESS: ";
	std::cerr << (addr & 0xff) << "."
    << ((addr & 0xff00)>>8) << "."
    << ((addr & 0xff0000)>>16) << "."
    << ((addr & 0xff000000)>>24);
	std::cerr << " PORT : " << ntohs(port);
	std::cerr << "***\n\n";
	std::cerr << std::flush;
}

int Server::startServer(int i)
{
	int socket_listen = socket(AF_INET, SOCK_STREAM, 0);
	int enable = 1;
	
	if (socket_listen < 0)
	{
		close(socket_listen);
		std::cerr << "Cannot create socket" << std::endl;
		return (1);
	}
	if (setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		close(socket_listen);
		std::cerr << "Error setting SO_REUSEADDR" << std::endl;
		return (1);
	}
	if (bind(socket_listen, (sockaddr *)&_socketAddresses[i], sizeof(_socketAddresses[i])) < 0)
	{

		close(socket_listen);
		std::cerr << "Cannot connect socket to address" << std::endl;
		return (1);
	}

	if (listen(socket_listen, 20) < 0)
	{
		close(socket_listen);
		throw std::runtime_error("Socket listen failed");
	}

	fcntl(socket_listen, F_SETFD, FD_CLOEXEC);
	printStartServer(_socketAddresses[i].sin_addr.s_addr, _socketAddresses[i].sin_port);
	_sockets_listen.push_back(socket_listen);
	return (0);
}

void Server::closeServer()
{
	for (unsigned int i = 0; i < _sockets_listen.size(); i++)
		close(_sockets_listen[i]);
	for (unsigned int i = 0; i < _clients.size(); i++)
		close(_clients[i].getFD());
}

void Server::initPollfds(std::vector<pollfd> *pollfds)
{
	for (unsigned int i = 0; i < _sockets_listen.size(); i++)
	{
        pollfd pfd;
        pfd.fd = _sockets_listen[i];
        pfd.events = POLLIN;
        pollfds->push_back(pfd);
    }
}

void Server::addPollfd(std::vector<pollfd> *pollfds, int client_fd, sockaddr_in client_addr, int i)
{
	pollfd new_pollfd = {0, 0, 0};

	new_pollfd.fd = client_fd;
	new_pollfd.events = POLLIN | POLLOUT;
	pollfds->push_back(new_pollfd);
	_clients.push_back(Client(client_fd, &_conf, _servers[i].getIP(), client_addr, _socketAddresses[i]));
	std::cerr << "[" << client_fd << "] New connection (CONN_COUNT="<<  _clients.size() << ")" << std::endl;
}

std::string Server::parseReferer(std::string referer)
{
	std::vector<std::string> out;

	split(referer, out, '/');
	if (out[out.size() - 1].find("html") != std::string::npos)
		return (out[out.size() - 1]);
	return ("");
}

void Server::loop()
{
	int 				ready;
	int 				client_fd = -1;
	std::vector<pollfd> pollfds;
	sockaddr_in			client_addr;
	ssize_t				pkt_len;
	char        		chunk[BUFFER_SIZE];
	
	initPollfds(&pollfds);
	while (true)
	{
		if (g_sig)
		{
			if (client_fd > 0)
				close(client_fd);
			break ;
		}
		ready = poll(pollfds.data(), pollfds.size(), 100);
		if (ready == -1)
		{
			if (errno == EINTR)
			{
				if (client_fd > 0)
					close(client_fd);
				break ;
			}
			std::cerr << "Poll failed." << std::endl;
			break ;
		}

		if (_clients.size() < MAX_CLIENTS)
		{
			for (unsigned int i = 0; i < _conf.getListens().size(); i++)
			{
				if (pollfds[i].revents & POLLIN)
				{
					client_addr = acceptConnection(client_fd, i);
					if (client_fd != -1)
						addPollfd(&pollfds, client_fd, client_addr, i);
				}
			}
		}
		for (size_t i = _conf.getListens().size(); i < pollfds.size(); i++)
		{
			size_t j = i - _conf.getListens().size();

			if (pollfds[i].revents & POLLIN)
			{
				pkt_len = recv(pollfds[i].fd, chunk, BUFFER_SIZE - 1, MSG_NOSIGNAL | MSG_DONTWAIT);
				if (pkt_len <= 0)
					_clients[j].close();
				if (_clients[j].getState() == Header || _clients[j].getState() == Body)
					_clients[j].receive(chunk, pkt_len);
			}
			if ((pollfds[i].revents & POLLOUT) &&
			(_clients[j].getState() == RespondingHeader || _clients[j].getState() == RespondingBody))
				_clients[j].respond();
			if (_clients[j].isExpired())
			{
				std::cerr << "[" << pollfds[i].fd << "] Closed connection.(CONN_COUNT=" <<  _clients.size() << ")" << std::endl;
				close(pollfds[i].fd);
				pollfds.erase(pollfds.begin() + i);
				_clients.erase(_clients.begin() + j);
			}
		}
	}
}

sockaddr_in	Server::acceptConnection(int &new_socket, int i)
{
	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	new_socket = accept(_sockets_listen[i], (sockaddr *)&client_addr, &client_len);
	if (new_socket == -1)
		std::cerr << "Server failed to accept incoming connection" << std::endl;
	else
		fcntl(new_socket, F_SETFD, FD_CLOEXEC);
	return client_addr;
}