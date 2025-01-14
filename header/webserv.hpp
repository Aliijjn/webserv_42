/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: lbartels <lbartels@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/01 16:20:54 by lbartels      #+#    #+#                 */
/*   Updated: 2024/11/11 13:15:10 by akuijer       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <array>
#include <string>
#include <chrono>
#include <fstream>
#include <cstring>
#include <csignal>
#include <sstream>
#include <iostream>
#include <filesystem>

#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "Server.hpp"
#include "ServerManager.hpp"

struct	t_location;

static constexpr bool	debug = false;

namespace log
{
	static std::ofstream	FD;
	static std::string		errorMsg;
	static bool				enable;

	void	setEnable(bool value);
	void	open(std::string name, ServerManager& serverManager);
	void	addMsg(std::string msg, bool error = true);
	void	write(Server& server, Request& request, Response& response, int clientFD);
	void	close();
}

// time
int64_t		getTimeMS();
bool		timer(int64_t durationMS = 0);

// tools
void			addFlag(int fd, int flag);
bool			isDir(std::string path);
std::string		findSegment(std::string request, std::string start, std::string end, uint64_t startIndex = 0);
std::string		getLocalTime(std::string format);
std::string		findKeyword(const std::string& haystack, const std::string& needle, bool throwError);
void			defaultPage(Response& response, std::string title, std::string text);

// lookup
std::string		getFullFileType(std::string fileType);
std::string 	getErrorCode(uint16_t code);

// run
void		handleGet(Server& server, Request& request, Response& response);
void		handlePost(Server& server, Request& request, Response& response);
void		handleDelete(Server& server, Request& request, Response& response);
void		handleCGI(Server& server, Request& request, Response& response);
void		getErrorPage(Server& server, Response& response);
void		parseRequest(std::string input, Server& server, Request& request, Response& response);

std::map<std::string, t_location>::iterator	smartFindLocation(Server& server, std::string fullPath);
