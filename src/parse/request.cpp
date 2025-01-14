#include "../../header/webserv.hpp"

void	fillBody(Request& request, Response& response, std::string input)
{
	std::string	boundary = findSegment(input, "boundary=", "\r\n", strlen("boundary="));
	const char*	endOfHeader = "\r\n\r\n";
	uint64_t	start;

	if (boundary == "")
	{
		start = input.find(endOfHeader);
		if (start == input.npos)
			goto error;
		request.body = input.substr(start + strlen(endOfHeader));
		return;
	}
	start = input.find(boundary);
	if (start == input.npos)
		goto bound;
	start = input.find(boundary, start + 1);
	if (start == input.npos)
		goto bound;
	request.body        = input.substr(start + boundary.length());
	request.fileName    = findSegment(request.body, "filename=\"",    "\"", strlen("filename=\""));
	request.contentType = findSegment(request.body, "Content-Type: ", "\r\n", strlen("Content-Type: "));
	request.body        = findSegment(request.body, endOfHeader, "--" + boundary, strlen(endOfHeader));
	return;

	bound:
	{
		log::addMsg("couldn't find boundary");
	}
	error:
	{
		log::addMsg("failed to read body");
		response.code = 400;
	}
}

void	parseRequest(std::string input, Server& server, Request& request, Response& response)
{
	if (input.length() == 0)
	{
		response.code = 400;
		return;
	}
	request.method = findSegment(input, "", " ");
	request.path   = findSegment(input, "/", " ");
	if (request.path.length() > 1 && isDir(server._path + request.path))
		request.path += "/";

	uint64_t	start = request.path.find('.');
	response.fileType = start == request.path.npos ? "" : request.path.substr(start);

	request.protocol   = findSegment(input, "HTTP", "\r\n");
	request.serverName = findSegment(input, "Host:", ":", strlen("Host: "));
	std::string	temp   = findSegment(input, request.serverName, "\r\n", strlen(request.serverName.c_str()) + 1);
	try
	{
		request.port = std::stoi(temp);
	}
	catch (std::exception& e)
	{
		log::addMsg("no valid port in request:");
		request.port = 0;
	}

	temp = findSegment(input, "Content-Length:", "\r\n", strlen("Content-Length: "));
	try
	{
		request.bodySize = std::stoi(temp);
	}
	catch (std::exception& e)
	{
		request.bodySize = 0;
	}

	if (request.method != "GET" && request.method != "POST" && request.method != "DELETE")
	{
		response.code = 501;
	}
	if (request.protocol != "HTTP/1.1")
	{
		response.code = 505;
	}
	if (response.code == 200 && request.bodySize > 0)
	{
		fillBody(request, response, input);
	}
}
