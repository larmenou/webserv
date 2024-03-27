/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rralambo <rralambo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 08:29:40 by larmenou          #+#    #+#             */
/*   Updated: 2024/03/27 15:50:31 by rralambo         ###   ########.fr       */
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

		HTTPError::initHTTPErrors();
		Server s = Server(conf);
		s.loop();
	} catch (std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
