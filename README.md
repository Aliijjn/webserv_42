# webserv_42

## What we learned from this project: ✅
- How the HTTP 1.1 protocol works
- Efficient use of sockets
- Efficient use of containers (std::map, std::vector)
- Advanced string manipulation in C++

## Project Goals: 🎯
- Build a webserver that can serve multiple websites
- Support GET, POST and DELETE methods
- Support [CGI](https://en.wikipedia.org/wiki/Common_Gateway_Interface) (remotely run scripts)
- Read server settings from a config file such as:
  - A servers host adress and port
  - The path where the server's files are located
  - The path to certain error pages
  - Seperate partitions where you can:
    - Set an index page (home page)
    - Set the allowed methods
    - Enable auto-indexing (very scary 👻)

## Images: 📷
### The homepage of our website hosted on our server:
![image](https://github.com/user-attachments/assets/4c510f61-a148-44c8-9449-637c8ef8b133)

### A beautiful auto-indexing page
![image](https://github.com/user-attachments/assets/ee13f76a-cb0b-4875-94a2-24c979fec8bf)

### Our webserver also supports images
![image](https://github.com/user-attachments/assets/6f3701c0-63bc-4942-9bc5-3de3162ed3e2)

### A CGI script that retrieves the time
![image](https://github.com/user-attachments/assets/2eaeff41-6178-47a0-86a1-1f3601c6855d)

## What a basic config file looks like: ⚙️
This config file is enough to run a website with some static pages in `/` (root) and in `/pages`.
It also supports the use of CGI scripts in `/cgi-bin` and uploads in `/www`.
```
ENABLE_LOG	true

SERVER	server1
{
	HOST			127.0.0.1
	PORT			8080
	PATH			./s1
	BODY_SIZE		4096
	TIME_OUT		1000 # in milliseconds

	LOCATION		/
	[
		INDEX			home.html
		PERMISSIONS		get
		TEMP_FILE		tempfile
	]
	LOCATION		/pages
	[
		AUTO_INDEX		true
		PERMISSIONS		get
	]
	LOCATION		/cgi-bin
	[
		INDEX			home.html
		PERMISSIONS		get
	]
	LOCATION		/www
	[
		INDEX			home.html
		PERMISSIONS		get, post, delete
	]
	LOCATION		/errorPages
	[
		PERMISSIONS		get
	]

	PAGE_201		errorPages/201.html
	PAGE_400		errorPages/400.html
	PAGE_413		errorPages/413.html
	PAGE_405		errorPages/405.html
}
```

## Log File 🗒️
The log file can be enabled with `ENABLE_LOG` in the config file.
If enabled, it'll print the adresses and ports of all servers upon startup.
It'll also print all incoming request and their response code, as well as any warnings or errors. 

Here's a basic example of the output of a log file:
```
Started at 2025-01-14 15:22:10
Running servers:
- server1 @ localhost:8080
- server2 @ 127.0.0.2:9090

server1 @ 15:22:19:
  client[3]
  GET /
  -> 200 OK

server1 @ 15:22:19:
  client[3]
  GET /style.css
  -> 200 OK

server1 @ 15:22:21:
  client[3]
  GET /test.html
  -> 200 OK

server1 @ 15:22:22:
  client[3]
  GET /style.css
  -> 200 OK

server1 @ 15:22:25:
  client[3]
  GET /cgi-bin/infinite.py
  ERROR: script timed out
  WARNING: using generic errorpage
  -> 500 Internal Server Error

server1 @ 15:22:48:
  client[3]
  POST /www/
  -> 413 Payload Too Large

server1 @ 15:22:48:
  client[3]
  GET /style.css
  -> 200 OK

server2 @ 15:23:04:
  client[3]
  GET /
  WARNING: using generic errorpage
  -> 404 Not Found

server2 @ 15:23:12:
  client[3]
  GET /home.html
  -> 200 OK

server2 @ 15:23:15:
  client[3]
  GET /style.css
  -> 200 OK

==--------------------------==
```
