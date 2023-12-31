#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "Logger.hpp"

class Socket {
	private:
		int listenfd;
		int port;
		struct sockaddr_in serv_addr;
		Logger logger;
	public:
		Socket();
		Socket(int port);
		~Socket();
		Socket(const Socket& source);
		Socket& operator =(const Socket& source);

		int setSocket();
		void setListenFd();
		void setPort(int port);
		void setServerAddr();

		int getListenFd();
		int getPort();
		struct sockaddr_in getServerAddr();
};

#endif
