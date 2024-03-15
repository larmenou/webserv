/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rralambo <rralambo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:29:40 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/15 12:15:02 by rralambo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Config.hpp"

int main (int ac, char **av) 
{
	
	if (ac != 2)
		return std::cerr << "Usage : ./webserv <.conf path>" << std::endl, 1;
	try {
		Config conf(av[1]);
		const ServerConf &server = conf.getServerFromHostAndIP("", "127.0.0.1");

		std::cout << "Found server " << server.getIP()  << ":" << server.getPort() << std::endl;
		std::cout << "404 error page at '" << server.getErrorPage(404) << "'" << std::endl;
		const Route &route = server.findRouteFromURN("/");
		std::cout << "Found route " << route.getRoute() << std::endl;
		const std::vector<ServerConf>   &servers = conf.getServers();
		for (size_t i = 0; i < servers.size(); i ++)
		{
			std::cout << servers[i] << std::endl;
		}

		Server s = Server(server.getIP(), server.getPort());
		s.startListen();
	} catch (std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}

	return (0);
}
