#include "Socket.hpp"

Socket::Socket() {}

Socket::Socket(int port): port(port) {}

Socket::~Socket() {
}

Socket::Socket(const Socket& source)  {
	*this = source;
}

Socket& Socket::operator =(const Socket& source) {
	if (this == &source)
		return *this;
	this->listenfd = source.listenfd;
	this->port = source.port;
	this->serv_addr = source.serv_addr;
	return *this;
}

void Socket::setListenFd() {
	this->listenfd = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(listenfd, F_SETFL, O_NONBLOCK);
	if (this->listenfd == -1) {
		std::cout << "socket() failed Error" << std::endl;
        std::exit(1);
	}
}

void Socket::setPort(int port) {
	this->port = port;
}

void Socket::setServerAddr() {
	std::memset(&this->serv_addr, 0, sizeof(this->serv_addr));

	this->serv_addr.sin_family = AF_INET;
	this->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->serv_addr.sin_port = htons(this->port);
}

int Socket::setSocket() {
	Socket::setListenFd();
	int optval = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		std::cout << "setsockopt() failed Error" << std::endl;
		close(listenfd);
		return -1;
	}

	Socket::setServerAddr();
	if (bind(this->listenfd, (struct sockaddr*)&this->serv_addr, sizeof(this->serv_addr)) == -1) {
		std::cout << "bind() faild.(" << errno << ") Error" << std::endl;
		close(this->listenfd);
		return -1;
	}
	if (listen(this->listenfd, SOMAXCONN) ==  -1) {
		std::cout << "listen() failed Error" << std::endl;
		close(this->listenfd);
		return -1;
	}
	return 0;
}

int Socket::getListenFd() {
	return listenfd;
}

int Socket::getPort() {
	return port;
}

struct sockaddr_in Socket::getServerAddr() {
	return serv_addr;
}
