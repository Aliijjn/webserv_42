#include "../../header/webserv.hpp"

void	defaultPage(Response& response, std::string title, std::string text)
{
	std::ifstream		infile("defaultPage.html", std::ios::in);
	std::stringstream	buffer;
	uint64_t			start;

	if (infile.is_open() == false)
	{
		response.code = 500;
		return;
	}
	buffer << infile.rdbuf();
	response.body = buffer.str();
	
	start = response.body.find("REPLACE1");
	if (start != response.body.npos)
	{
		response.body.replace(start, strlen("REPLACE1"), title);
	}
	start = response.body.find("REPLACE2");
	if (start != response.body.npos)
	{
		response.body.replace(start, strlen("REPLACE2"), text);
	}

	response.bodySize = response.body.length();
}

void	getErrorPage(Server& server, Response& response)
{
	auto				map = server._errorTable.find(response.code);
	std::string			filePath;
	std::ifstream		infile;
	std::stringstream	buffer;

	if (map == server._errorTable.end())
	{
		log::addMsg("using generic errorpage", false);
		std::string	errorCode =  getErrorCode(response.code);
		defaultPage(response, errorCode, "<h1>" + errorCode + "</h1>");
		return;
	}
	filePath = server._path + "/" +  map->second;
	infile.open(filePath, std::ios::in);
	if (infile.is_open() == false)
	{
		response.code = 500;
		return;
	}
	buffer << infile.rdbuf();
	response.body = buffer.str();
	response.bodySize = response.body.length();
}

void	autoIndex(Server& server, Request& request, Response& response)
{
	namespace fs = std::filesystem;

	log::addMsg("using autoindex", false);
	std::string	list = "<h1>directory: " + request.path + "</h1>\r\n";
	fs::path	listedDir(server._path + request.path);
	for (const auto& it : fs::directory_iterator(listedDir))
	{
		std::string name(it.path().c_str());
		std::string link(name.c_str() + server._path.length());
		std::string displayName(name.c_str() + server._path.length() + request.path.length());
		list += "<a href=\"" + link + "\"><h2>" + displayName + "</h2></a>\r\n";
	}
	defaultPage(response, "Directory Listing", list);
}

void	handleGet(Server& server, Request& request, Response& response)
{
	auto			location = smartFindLocation(server, request.path);
	std::string		path;
	std::ifstream	infile;
	bool			defaultPage = false;

	if (location == server._locations.end())
	{
		response.code = 403;
	}
	else if (location->second.allowGet == false)
	{
		response.code = 405;
	}
	if (response.code == 200) // get path and open file (checking if index should be added doesnt work!!)
	{
		if (location->first.length() == request.path.length())
		{
			defaultPage = true;
			path = server._path + location->first + location->second.index;
		}
		else
		{
			path = server._path + request.path;
		}
		infile.open(path, std::ios::binary | std::ios::in);
		if (infile.is_open() == false)
		{
			if (defaultPage == true && location->second.autoIndex == true)
				return autoIndex(server, request, response);
			response.code = 404;
		}	
	}

	if (response.code == 200) // read file if file is found
	{
		infile.seekg(0, std::ios::end);
		response.bodySize = infile.tellg();
		std::string	cpy(response.bodySize, '\0');
		infile.seekg(0, std::ios::beg);
		infile.read(&cpy[0], response.bodySize);
		response.body = cpy;
		infile.close();
		response.bodySize = response.body.length();
	}
}
