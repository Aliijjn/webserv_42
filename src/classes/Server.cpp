#include "../../header/Server.hpp"
#include "../../header/webserv.hpp"

std::map<uint16_t, std::string>	getErrorPages(const std::string& haystack)
{
	std::map<uint16_t, std::string>	errorTable;
	const std::string				needle = "PAGE_";
	uint64_t						start = 0;
	uint64_t						end;

	while (true)
	{
		start = haystack.find(needle, start);
		end = haystack.find_first_of(" \t", start);
		if (start == haystack.npos || end == haystack.npos)
			break;
		start += needle.length();
		uint16_t errorCode = (uint16_t)std::stoi(haystack.substr(start, end - start));
		start = haystack.find_first_not_of(" \t", end);
		end = haystack.find_first_of("\n", start);
		if (start == haystack.npos || end == haystack.npos)
		{
			std::cerr << "couldn't find path to error page " << errorCode << std::endl;
			throw Server::ParseError();
		}
		std::string errorStr = haystack.substr(start, end - start);
		if (errorTable.find(errorCode) != errorTable.end())
		{
			std::cerr << "duplicate page for " << errorCode << "\n";
			throw Server::ParseError();
		}
		errorTable.insert(std::pair<uint16_t, std::string>(errorCode, errorStr));
	}
	return errorTable;
}

void	validateLocations(std::map<std::string, t_location>& table)
{
	auto	root = table.find("/");
	if (root == table.end())
	{
		std::cerr << "Root \"/\" not found in config file\n";
		throw Server::ParseError();
	}

	if (root->second.index == "")
		root->second.index = "index.html";
	if (root->second.tempFile == "")
		root->second.tempFile = "tempfile";

	for (auto& it : table)
	{
		if (it.second.index == "")
			it.second.index = root->second.index;
		if (it.second.tempFile == "")
			it.second.tempFile = root->second.tempFile;
	}
}

std::map<std::string, t_location>	getLocations(const std::string& haystack)
{
	std::map<std::string, t_location>	table;
	uint64_t							start = 0;
	uint64_t							end = 0;
	std::string							substr;
	t_location							location;
	std::string							temp;

	while (true)
	{
		start = haystack.find("LOCATION", end);
		end = haystack.find_first_of("]", end + 1);
		if (start == haystack.npos ^ end == haystack.npos)
		{
			std::cerr << "invalid location syntax\n";
			throw Server::ParseError();
		}
		if (start == haystack.npos || end == haystack.npos)
		{
			break;
		}
		substr = haystack.substr(start, end - start);

		temp = findKeyword(substr, "PERMISSIONS", false);
		location.allowGet    = temp.find("get")    != temp.npos;
		location.allowPost   = temp.find("post")   != temp.npos;
		location.allowDelete = temp.find("delete") != temp.npos;
		location.autoIndex = findKeyword(substr, "AUTO_INDEX", false) == "true";
		location.index     = findKeyword(substr, "INDEX", false);
		location.tempFile  = findKeyword(substr, "TEMP_FILE", false);
		temp = findKeyword(substr, "LOCATION", true);
		if (temp == "")
		{
			std::cerr << "location is empty\n";
			throw Server::ParseError();
		}
		if (temp.length() > 1)
			temp += '/';

		if (table.find(temp) != table.end())
		{
			std::cerr << "duplicate location for " << temp << "\n";
			throw Server::ParseError();
		}
		table.insert(std::pair<std::string, t_location>(temp, location));
	}
	validateLocations(table);
	return table;
}

void	throwError(const char* msg, int serverFD, addrinfo* serverAddress)
{
	if (serverAddress != nullptr)
		freeaddrinfo(serverAddress);
	perror(msg);
	close(serverFD);
	throw Server::InitError();
}

void	Server::setUp()
{
	addrinfo	hints{};
	addrinfo*	serverAddress;

	_opt = 1;
	hints = {
		.ai_flags = AI_PASSIVE,
		.ai_family = AF_INET, 
		.ai_socktype = SOCK_STREAM
	};
	if (getaddrinfo(_host.c_str(), std::to_string(_port).c_str(), &hints, &serverAddress) < 0)
	{
		throwError("getaddrinfo", _serverFD, nullptr);
	}
	_serverFD = socket(serverAddress->ai_family, serverAddress->ai_socktype, serverAddress->ai_protocol);
	if (_serverFD < 0)
	{
		throwError("socket", _serverFD, serverAddress);
	}
	// allows server to immideatly be reused
	if (setsockopt(_serverFD, SOL_SOCKET, SO_REUSEADDR, &_opt, sizeof(int)))
	{
		throwError("setsockopt", _serverFD, serverAddress);
	}
	if (bind(_serverFD, serverAddress->ai_addr, serverAddress->ai_addrlen) < 0)
	{
		throwError("bind", _serverFD, serverAddress);
	}
	if (listen(_serverFD, 10) < 0)
	{
		throwError("listen", _serverFD, serverAddress);
	}
	freeaddrinfo(serverAddress);
}

template <typename T>
void	findVal(const std::string& config, const std::string& keyword, T& value)
{
	try
	{
		std::string	str = findKeyword(config, keyword, false);
		if (str != "")
		{
			int64_t	temp = std::stol(str);
			value = (T)temp;
			if (temp < 0 || (int64_t)value != temp)
			{
				throw std::bad_cast();
			}
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << "Failed to parse " << keyword << " number: " << e.what() << '\n';
		throw Server::ParseError();
	}
}

Server::Server(const std::string& config)
{
	static	uint32_t serverCount;

	findVal(config, "PORT", _port);
	findVal(config, "BODY_SIZE", _bodySize);
	findVal(config, "TIME_OUT", _timeOut);
	_name = findKeyword(config, "SERVER", true);
	if (_name == "")
		_name = "server" + std::to_string(++serverCount);
	_host = findKeyword(config, "HOST", true);
	_path = findKeyword(config, "PATH", false);
	if (_path == "")
		_path = ".";
	_errorTable = getErrorPages(config);
	_locations = getLocations(config);
	setUp();
}

Server::~Server(){}

const char* Server::ParseError::what() const throw()
{
	return ("Error parsing configuration file");
}

const char* Server::InitError::what() const throw()
{
	return ("Error while initialising connection");
}

std::ostream&	operator<<(std::ostream& out, Server& server)
{
	out << "\n\nSERVER\t" << server._name << "\n{" <<
	"\n\tHOST\t\t" << server._host <<
	"\n\tPORT\t\t" << server._port <<
	"\n\tBODY_SIZE\t" << server._bodySize <<
	"\n\tPATH\t\t" << server._path << 
	"\n\tTIME_OUT\t" << server._timeOut << "ms\n\n";

	for (auto it : server._locations)
	{
		out << "\tLOCATION\t" << it.first <<
		"\n\t[\n\t\tINDEX\t\t" << it.second.index <<
		"\n\t\tAUTO_INDEX\t" << (it.second.autoIndex == true ? "true" : "false") <<
		"\n\t\tPERMISSIONS\t" << 
		(it.second.allowGet    == true ? "get " : "") <<
		(it.second.allowPost   == true ? "post " : "") <<
		(it.second.allowDelete == true ? "delete " : "") <<
		"\n\t\tTEMP_FILE\t" << it.second.tempFile << "\n\t]\n";
	}
	std::cout << "\n";
	for (auto it : server._errorTable)
	{
		out << "\tPAGE_" << it.first << "\t" << it.second << "\n";
	}
	std::cout << "}\n";
	return out;
}