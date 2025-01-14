#include "../../header/webserv.hpp"

void	addFlag(int fd, int flag)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1 || fcntl(fd, F_SETFL, flags | flag) == -1)
	{
		perror("addFlag failed");
		return;
	}
}

bool	isDir(std::string path)
{
	struct stat		path_stat;

	// Get information about the path
	if (stat(path.c_str(), &path_stat) == -1)
	{
		return false;
	}
	// Check if it's a directory
	return S_ISDIR(path_stat.st_mode);
}

std::map<std::string, t_location>::iterator	smartFindLocation(Server& server, std::string fullPath)
{
	uint64_t	secondSlash = fullPath.find_last_of("/");
	std::string	location = fullPath.substr(0, secondSlash + 1);

	return server._locations.find(location);
}

std::string	findSegment(std::string request, std::string start, std::string end, uint64_t startIndex)
{
	uint64_t	temp = request.find(start);
	if (temp >= request.npos)
		return "";
	startIndex += temp;

	u_int64_t	endIndex = request.find(end, startIndex);
	if (endIndex == request.npos)
		return "";
	
	return request.substr(startIndex, endIndex - startIndex);
}

std::string	getLocalTime(std::string format)
{
	auto				now = std::chrono::system_clock::now();
	std::time_t			currentTime = std::chrono::system_clock::to_time_t(now);
	std::tm				localTime = *std::localtime(&currentTime);
	std::ostringstream	dateStream;

	dateStream << std::put_time(&localTime, format.c_str());
	return dateStream.str();
}

int64_t	getTimeMS()
{
	using namespace std::chrono;

	return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

// Set by calling with `durationMS` set to the desired duration
// Will otherwise return true when time has passed
bool	timer(int64_t durationMS)
{
	static int64_t endTimeMS;
	int64_t currentTimeMS = getTimeMS();

	if (durationMS != 0)
	{
		endTimeMS = currentTimeMS + durationMS;
		return false;
	}
	return endTimeMS <= currentTimeMS;
}

std::string	findKeyword(const std::string& haystack, const std::string& needle, bool throwError)
{
	uint64_t start = haystack.find(needle);
	uint64_t end = haystack.find_first_of("\n#", start);
	if ((start == haystack.npos || end == haystack.npos))
	{
		if (throwError == true)
		{
			std::cerr << "Couldn't find " << needle << std::endl;
			throw Server::ParseError();
		}
		return "";
	}
	start += needle.length();
	start = haystack.find_first_not_of(" \t", start);
	while (end > 0 && (haystack[end - 1] == ' ' || haystack[end - 1] == '\t'))
		end--;
	if (start > end)
	{
		std::cerr << "findKeyword: start > end\ncheck if " << needle << " has a value\n";
		throw Server::ParseError();
	}
	return haystack.substr(start, end - start);
}