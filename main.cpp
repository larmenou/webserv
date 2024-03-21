/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmenou <larmenou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:29:40 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/20 13:57:32 by larmenou         ###   ########.fr       */
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
		const std::vector<ServerConf>   &servers = conf.getServers();

		HTTPError::initHTTPErrors();
		Server s = Server(servers);
		s.loop();
	} catch (std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}

	return (0);
}
