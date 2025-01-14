#include "../../header/webserv.hpp"

std::string	findValidTempfile(std::string path)
{
	namespace fs = std::filesystem;

	std::string	fullPath;

	for (uint16_t i = 1; i != 0; i++)
	{
		fullPath = path + "_" + std::to_string(i);
		if (fs::exists(fullPath) == false)
		{
			return fullPath;
		}
	}
	return "";
}

void	handlePost(Server& server, Request& request, Response& response)
{
	auto			location = smartFindLocation(server, request.path);
	std::string		path;

	if (location == server._locations.end())
	{
		response.code = 404;
	}
	else if (location->second.allowPost == false)
	{
		response.code = 405;
	}
	if (response.code != 200)
	{
		return;
	}
	path = server._path + request.path + request.fileName;
	if (isDir(path) == true)
	{
		log::addMsg("made tempfile", false);
		path = server._path + location->first + server._locations[location->first].tempFile;
		path = findValidTempfile(path);
		if (path == "")
		{
			log::addMsg("tempfile count exceeds " + std::to_string(UINT16_MAX));
			response.code = 500;
			return;
		}
	}
	std::ofstream	outfile(path);
	if (outfile.is_open() == false)
	{
		log::addMsg("couldn't open " + path);
		response.code = 404;
		return;
	}
	outfile << request.body;
	outfile.close();
	response.code = 201;
}