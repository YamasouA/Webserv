#include "Epoll.hpp"

Epoll::Epoll() {
	epfd = epoll_create(10); //tmp size
	if (epfd == -1) {
		std::cerr << "epoll_create Error" << std::endl;
		std::exit(1);
	}
}

Epoll::Epoll(const Epoll& src)
:epfd(src.epfd)
{}

Epoll& Epoll::operator=(const Epoll& rhs) {
	if (this == &rhs) {
		return *this;
	}
	this->epfd = rhs.epfd;
	return *this;
}

Epoll::~Epoll() {

}

int Epoll::setEvent(int fd, uint32_t event, int op) {
	struct epoll_event register_event;

	std::memset(&register_event, 0, sizeof(register_event));
	register_event.data.fd = fd;
	register_event.events = event;
	if (epoll_ctl(epfd, op, fd, &register_event)) {
		std::cerr << "errno: " << errno << std::endl;
		perror("epoll Error(register)");
		return -1;
	}
	return 0;
}

int Epoll::getEventsNum() {
	int event_num = epoll_wait(epfd, reciver_event, 10, kTimeout);
	if (event_num == -1) {
		std::cerr << "errno: " << errno << std::endl;
		perror("epoll Error(reciver)");
		return -1;
	}
	return event_num;
}

struct epoll_event* Epoll::getReciverEvent() {
	return reciver_event;
}
