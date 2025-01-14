#include "../../header/Server.hpp"

void	free2D(const char* const* envArray)
{
	for (uint32_t i = 0; envArray && envArray[i] != nullptr; i++)
	{
		delete[] envArray[i];
	}
	delete[] envArray;
}

const char**	initEnv(const Request& request)
{
	std::vector<std::string> env = 
	{
		"REDIRECT_STATUS=200",
		"GATEWAY_INTERFACE=CGI/1.1",
		"PATH=/usr/bin:/bin",
		"REQUEST_METHOD=" + request.method,
		"SERVER_PROTOCOL=" + request.protocol,
		"SERVER_NAME=" + request.serverName,
		"SERVER_PORT=" + std::to_string(request.port),
		"CONTENT_TYPE=" + request.contentType,
		"CONTENT_LENGTH=" + std::to_string(request.bodySize),
		"SCRIPT_NAME=" + request.path,
		"SCRIPT_FILENAME=" + request.path,
		"PATH_INFO=" + request.path,
		"TZ=Europe/Amsterdam"
	};

	char**	envArray{};
	try
	{
		envArray = new char*[env.size() + 1]{};
		for (uint32_t i = 0; i < env.size(); i++)
		{
			envArray[i] = new char[env[i].length() + 1];
			std::strcpy(envArray[i], env[i].c_str());
		}
	}
	catch(const std::exception& e)
	{
		free2D(envArray);
		std::cerr << "new failed when making env";
		return nullptr;
	}
	
	return (const char**)envArray;
}

bool	initFork(int in[2], int out[2], pid_t& pid)
{
	if (pipe(in) < 0)
	{
		log::addMsg("pipe error");
		return false;
	}
	if (pipe(out) < 0)
	{
		close(in[0]);
		close(in[1]);
		log::addMsg("pipe error");
		return false;
	}
	pid = fork();
	if (pid == -1)
	{
		close(in[0]);
		close(in[1]);
		close(out[0]);
		close(out[1]);
		log::addMsg("fork error");
		return false;
	}
	return true;
}

void	execute(Server& server, Request& request, int in[2], int out[2])
{
	close(in[1]);
	close(out[0]);

	std::string		path    = server._path + request.path;
	const char**	env     = initEnv(request);
	const char*		argv[2] = {path.c_str(), nullptr};

	if (env == nullptr)
	{
		exit(EXIT_FAILURE);
	}
	if (dup2(in[0], STDIN_FILENO) < 0 || dup2(out[1], STDOUT_FILENO) < 0)
	{
		exit(EXIT_FAILURE);
	}
	execve(argv[0], (char* const*)argv, (char* const*)env);
	free2D(env);
	exit(EXIT_FAILURE);
}

void	handleCGI(Server& server, Request& request, Response& response)
{
	int		in[2], out[2];
	pid_t	pid;
	
	if (isDir(server._path + request.path))
	{
		response.code = 403;
		return;
	}
	if (initFork(in, out, pid) == false)
	{
		response.code = 500;
		return;
	}
	if (!pid)
	{
		execute(server, request, in, out);
	}

	close(in[0]);
	close(out[1]);
	if (request.bodySize > 0)
	{
		if (write(in[1], request.body.c_str(), request.bodySize) < 0)
		{
			close(in[1]);
			close(out[0]);
			response.code = 500;
			log::addMsg("couldn't write to pipe");
			return;
		}
	}

	// check child
	close(in[1]);
	int	statusCode;
	timer(server._timeOut);
	while (waitpid(pid, &statusCode, WNOHANG) == 0)
	{
		if(timer() == true)
		{
			kill(pid, SIGKILL);
			response.code = 500;
			log::addMsg("script timed out");
			close(out[0]);
			return;
		}
	}
	if (WEXITSTATUS(statusCode) != 0)
	{
		response.code = 500;
		log::addMsg("failed to run script");
		close(out[0]);
		return;
	}

	// read
	char		buffer[1025]{};
	ssize_t		bytesRead;
	std::string	content;
	do
	{
		bytesRead = read(out[0], buffer, sizeof(buffer) - 1);
		content += buffer;
		std::memset(buffer, '\0', sizeof(buffer));
	}
	while(bytesRead > 0);
	close(out[0]);
	if (bytesRead < 0)
	{
		response.code = 500;
		log::addMsg("failed to read script output");
		return;
	}
	defaultPage(response, request.path, content);
	response.bodySize = response.body.length();
}
