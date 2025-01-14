#include "../../header/webserv.hpp"
#include "../../header/Server.hpp"
#include "../../header/ServerManager.hpp"

bool isRunning = true;

void sig_handle(int signal)
{
	(void)signal;
	isRunning = false;
}

Server *ServerManager::findServer(int eventIndex)
{
	for (auto it : _servers)
	{
		if (_events[eventIndex].data.fd == it->_serverFD)
		{
			return it;
		}
	}
	return nullptr;
}

bool	ServerManager::acceptNewConnection(Server &server)
{
	int			clientFD = accept(server._serverFD, nullptr, nullptr);
	Request		request;
	Response	response;

	if (clientFD == -1)
	{
		std::cerr << "accept failed miserably\n";
		return false;
	}
	addFlag(clientFD, O_NONBLOCK);

	_ev.events = EPOLLIN;
	_ev.data.fd = clientFD;

	if (epoll_ctl(_epFD, EPOLL_CTL_ADD, clientFD, &_ev) == -1)
	{
		std::cerr << "epoll_ctl failed\n";
		close(clientFD);
		return false;
	}
	_clientTable.insert(std::pair<int, Connection>(clientFD, {server, request, response}));
	return true;
}

uint16_t ServerManager::readRequest(std::string &str, Server &server, int clientFD)
{
	const char		*endOfHeader = "\r\n\r\n";
	char			buffer[1025]{};
	std::string		temp;
	ssize_t			readBytes = 0;
	ssize_t			totalBytesRead = 0;
	int64_t			bodySize = 0;

	timer(server._timeOut);
	do
	{
		readBytes = read(clientFD, buffer, 1024);
		if (readBytes <= 0)
			return 500;
		str.append(buffer, readBytes);
	}
	while (str.find(endOfHeader) == str.npos && timer() == false);
	temp = findSegment(str, "Content-Length:", "\n", strlen("Content-Length: "));
	if (temp != "")
	{
		try
		{
			if (temp.length() > 18)
				throw std::out_of_range("too large");
			bodySize = std::stol(temp);
			if (bodySize < 0)
				throw std::out_of_range("is negative");
		}
		catch (std::exception &e)
		{
			log::addMsg((std::string) "invalid content length: " + e.what());
			return 400;
		}
	}
	else
	{
		bodySize = 0;
	}
	totalBytesRead += str.length() - (str.find(endOfHeader) + strlen(endOfHeader));

	// read the entire request, even in case of a 413
	while (totalBytesRead < bodySize && timer() == false)
	{
		readBytes = read(clientFD, buffer, 1024);
		if (readBytes > 0)
		{
			totalBytesRead += readBytes;
			str.append(buffer, readBytes);
		}
		if (debug)
			std::cout << "read from client: \n\n"
					  << str << "end" << std::endl;
	}
	if ((uint64_t)bodySize > server._bodySize)
	{
		return 413;
	}
	if (timer() == true)
	{
		return 504;
	}
	return 200;
}

void	ServerManager::getHttpRequest(std::map<int, Connection>::iterator client)
{
	int			clientFD = client->first;
	Server&		server = client->second.server;
	Request&	request = client->second.request;
	Response&	response = client->second.response;
	std::string	buffer;

	response.code = readRequest(buffer, server, clientFD);
	if (response.code == 500)
	{
		epoll_ctl(_epFD, EPOLL_CTL_DEL, clientFD, NULL);
		close(clientFD);
		_clientTable.erase(clientFD);
		return;
	}
	parseRequest(buffer, server, request, response);
	if (response.code == 200)
	{
		if (request.path.find("/cgi-bin") == 0)
			handleCGI(server, request, response);
		else if (request.method == "POST")
			handlePost(server, request, response);
		else if (request.method == "GET")
			handleGet(server, request, response);
		else if (request.method == "DELETE")
			handleDelete(server, request, response);
	}
	if (response.code != 200)
	{
		getErrorPage(server, response);
	}
	log::write(server, request, response, clientFD);
	_ev.events = EPOLLOUT;
	_ev.data.fd = clientFD;
	epoll_ctl(_epFD, EPOLL_CTL_MOD, clientFD, &_ev);
}

void	ServerManager::sendHttpResponse(std::map<int, Connection>::iterator client)
{
	int			clientFD = client->first;
	Response	response = client->second.response;
	std::string	httpResponse;

	httpResponse = "HTTP/1.1 " + getErrorCode(response.code) + "\r\n";
	httpResponse += "Content-Type: " + getFullFileType(response.fileType) + "\r\n";
	httpResponse += "Content-Length: " + std::to_string(response.bodySize) + "\r\n";
	httpResponse += "Connection: " + (std::string)(response.code < 400 ? "keep-alive" : "close") + "\r\n";
	httpResponse += "\r\n";
	httpResponse += response.body;

	if (send(clientFD, httpResponse.c_str(), httpResponse.length(), 0) == -1)
	{
		log::addMsg("send");
	}
	if (epoll_ctl(_epFD, EPOLL_CTL_DEL, clientFD, NULL) == -1)
	{
		log::addMsg((std::string)"epoll_ctl fd: " + std::to_string(clientFD));
	}
	if (close(clientFD) == -1)
	{
		log::addMsg("close");
	}
	_clientTable.erase(clientFD);
}

void	ServerManager::run()
{
	std::cout << "\nRunning servers...\n";

	signal(SIGINT, sig_handle);

	while (isRunning)
	{
		int evCount = epoll_wait(_epFD, _events, 10, -1);
		if (evCount == -1)
		{
			log::addMsg("epoll_wait");
			continue;
		}

		for (int i = 0; i < evCount; i++)
		{
			Server *server = findServer(i);

			if (server != nullptr)
			{
				if (acceptNewConnection(*server) == false)
					continue;
			}
			else
			{
				auto	it = _clientTable.find(_events[i].data.fd);

				if (it != _clientTable.end() && _events[i].events & EPOLLIN)
				{
					getHttpRequest(it);
				}
			}
			if (_events[i].events & EPOLLOUT)
			{
				sendHttpResponse(_clientTable.find(_events[i].data.fd));
			}
		}
	}
	std::cout << "\nstopped running\n";
}
