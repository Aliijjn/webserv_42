/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerManager.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: akuijer <akuijer@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/11 16:49:06 by akuijer       #+#    #+#                 */
/*   Updated: 2024/12/17 14:47:25 by akuijer       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../header/Server.hpp"
#include "../../header/ServerManager.hpp"

ServerManager::ServerManager(const std::string& path)
{
	try
	{
		std::ifstream infile(path, std::ios::in);
		if (infile.is_open() == false)
		{
			std::cerr << "failed to open file: " << path << std::endl;
			throw ConfFileNotFound();
		}
		std::string str;
		std::getline(infile, str, '\0');
		if (str.empty() == true)
		{
			std::cerr << "failed to open file: " << path << std::endl;
			throw ConfFileNotFound();
		}
		
		uint64_t start = 0;
		uint64_t end = 0;

		log::setEnable(findKeyword(str, "ENABLE_LOG", false) == "true");
		while (true)
		{
			start = end;
			end = str.find('}', end);
			if (end == str.npos)
				break;
			end += 1;
			Server*	server = new Server(str.substr(start, end - start));
			_servers.push_back(server);
		}
		if (_servers.size() == 0)
		{
			std::cerr << "No servers found\n";
			throw Server::ParseError();
		}
		_epFD = epoll_create1(0);
		for (auto it : _servers)
		{
			_ev.events = EPOLLIN;
			_ev.data.fd = it->_serverFD;
			epoll_ctl(_epFD, EPOLL_CTL_ADD, it->_serverFD, &_ev);
		}
		log::open("log.txt", *this);
	}
	catch(const std::exception& e)
	{
		for (auto it : _servers)
		{
			delete it;
		}
		std::cerr << e.what() << "\n";
		throw std::exception();
	}
}

ServerManager::~ServerManager()
{
	for (auto it : _servers)
	{
		if (it->_serverFD != -1)
		{
			if (setsockopt(it->_serverFD, SOL_SOCKET, SO_REUSEADDR, &it->_opt, sizeof(it->_opt)) < 0)
			{
				perror("setsockopt failed");
			}
			close(it->_serverFD);
		}
	}
	for (auto it : _servers)
	{
		delete it;
	}
	log::close();
}

const char* ServerManager::ConfFileNotFound::what() const throw()
{
	return ("Error opening/reading");
}

std::ostream&	operator<<(std::ostream& out, ServerManager& serverManager)
{
	for (Server* it : serverManager._servers)
	{
		out << *it;
	}
	return out;
}