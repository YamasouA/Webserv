#include "Socket.hpp"

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

void Socket::set_listenfd() {
	this->listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->listenfd == -1) {
		std::cout << "socket() failed Error" << std::endl;
		exit(1);
	}
}

void Socket::set_port(int port) {
	this->port = port;
}

void Socket::set_serv_addr() {
	memset(&this->serv_addr, 0, sizeof(this->serv_addr));

	this->serv_addr.sin_family = AF_INET;
	this->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->serv_addr.sin_port = htons(this->port);
}

int Socket::set_socket() {
	Socket::set_listenfd();
	int optval = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		std::cout << "setsockopt() failed Error" << std::endl;
		close(listenfd);
		return -1;
	}

	Socket::set_serv_addr();
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

int Socket::get_listenfd() {
	return listenfd;
}

int Socket::get_port() {
	return port;
}

struct sockaddr_in Socket::get_serv_addr() {
	return serv_addr;
}
