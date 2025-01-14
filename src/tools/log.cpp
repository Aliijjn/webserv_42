#include "../../header/webserv.hpp"

void	log::setEnable(bool value)
{
	enable = value;
}

void	log::open(std::string name, ServerManager& serverManager)
{
	if (enable == false)
		return;
	if (std::filesystem::exists(name) == true)
	{
		FD.open(name, std::ios::app);
	}
	else
	{
		FD.open(name, std::ios::trunc);
	}
	if (FD.is_open() == false)
	{
		std::cerr << "Failed to open log file\n";
	}
	
	FD << "Started at " << getLocalTime("%Y-%m-%d %H:%M:%S") << "\nRunning servers:\n";
	for (Server* server : serverManager._servers)
	{
		FD << "- " << server->_name << " @ " << (server->_host == "127.0.0.1" ? "localhost" : server->_host) << ":" << server->_port << "\n";
	}
	FD << std::endl;
}

void	log::addMsg(std::string msg, bool error)
{
	if (enable == false)
		return;
	if (error == false)
		errorMsg += "  WARNING: " + msg + '\n';
	else
		errorMsg += "  ERROR: " + msg + '\n';
}

void	log::write(Server& server, Request& request, Response& response, int clientFD)
{
	if (enable == false)
		return;
	FD	<< server._name << " @ " << getLocalTime("%H:%M:%S") << ":\n"
		<< "  client[" << clientFD << "]\n"
		<< "  " << (request.method == "" ? "EMPTY REQUEST" : request.method + " " + request.path) << "\n"
		<< errorMsg
		<< "  -> " << getErrorCode(response.code)
		<< '\n' << std::endl;
	errorMsg.clear();
}

void	log::close()
{
	if (enable == false)
		return;
	FD << "==--------------------------==\n" << std::endl;
	FD.close();
}
