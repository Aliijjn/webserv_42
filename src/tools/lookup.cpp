#include "../../header/webserv.hpp"

std::map<uint16_t, std::string>	errorTable = {
	{200, "200 OK"},					{201, "201 Created"},			{204, "204 No Content"},
	{400, "400 Bad Request"},			{403, "403 Forbidden"},			{404, "404 Not Found"}, 		{405, "405 Method Not Allowed"}, 		{408, "408 Request Timeout"}, {413, "413 Payload Too Large"},
	{500, "500 Internal Server Error"},	{501, "501 Not Implemented"},	{504, "504 Gateway Timeout"}, 	{505, "505 HTTP Version Not Supported"}
};

std::string getErrorCode(uint16_t code)
{
	auto	errorCode = errorTable.find(code);

	if (errorCode == errorTable.end())
		return "500 Internal Server Error";
	return errorCode->second;
}

std::map<std::string, std::string>	fileTable = {
	{".txt", "text/plain"}, 	{".html", "text/html"},		{".css", "text/css"},
	{".png", "image/png"}, 		{".bmp", "image/bmp"},
	{".js", "text/javascript"}
};

std::string	getFullFileType(std::string fileType)
{
	auto	fullType = fileTable.find(fileType);

	if (fullType == fileTable.end())
		return "text/html";
	return fullType->second;
}
