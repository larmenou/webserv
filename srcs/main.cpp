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
	while (true)
	{
		try {
			Config conf(av[1]);
			HTTPError::initHTTPErrors();
			Server s(conf);

			try {
				s.loop();
				return EXIT_SUCCESS;
			} catch (std::exception &e)
			{
				std::cerr << "Error : " << e.what() << std::endl;
				if (std::strcmp(e.what(), "Failed to execute CGI.") == 0)
					return EXIT_FAILURE;
				std::cerr << "Restarting server ..." << std::endl;
				continue;
			}
		} catch (std::exception &e)
		{
			std::cerr << "Error : " << e.what() << std::endl;
			return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}
