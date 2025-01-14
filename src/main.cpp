#include "../header/webserv.hpp"

int	main(int argc, char *argv[])
{
	if (argc > 2)
	{
		std::cerr << "Argument count shouldn't exceed 1\nTry \"./webserv\" or \"./webserv path\"\n";
		return 1;
	}
	try
	{
		if (std::filesystem::exists("defaultPage.html") == false)
		{
			std::cerr << "Couldn't find dependency: \"defaultPage.html\"\n";
			throw Server::ParseError();
		}
		ServerManager serverManager(argc == 1 ? "configs/default.conf" : argv[1]);
		if (debug)
		{
			std::cout << serverManager;
		}
		serverManager.run();
	}
	catch(const std::exception& e)
	{
		return 1;
	}
	return 0;
}
