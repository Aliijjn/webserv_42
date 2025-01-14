#include "../../header/webserv.hpp"

void	handleDelete(Server& server, Request& request, Response& response)
{
	auto		location = smartFindLocation(server, request.path);
	std::string	path;

	if (location == server._locations.end())
	{
		response.code = 404;
	}
	else if (location->second.allowDelete == false)
	{
		response.code = 405;
	}
	if (response.code != 200)
	{
		return;
	}
	path = server._path + request.path;
	if (isDir(path) == true)
	{
		response.code = 403;
		return; //forbidden to delete directory
	}
	if (std::remove(path.c_str()) < 0)
	{
		response.code = 404;
	}
	else
	{
		response.code = 204;
	}
}