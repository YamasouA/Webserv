#include "Kqueue.hpp"

Kqueue::Kqueue()
{
	kq = kqueue();
	if (kq < -1) {
		std::cerr << "kqueue Error" << std::endl;
        std::exit(1);
	}
	time_over.tv_sec = 10;
	time_over.tv_nsec = 0;
}

Kqueue::Kqueue(const Kqueue& src) {
    this->changes = src.changes;
    this->kq = src.kq;
    this->time_over = src.time_over;
}

Kqueue& Kqueue::operator=(const Kqueue& rhs) {
    if (this == &rhs) {
        return *this;
    }
    this->changes = rhs.changes;
    this->kq = rhs.kq;
    this->time_over = rhs.time_over;
    return *this;
}

Kqueue::~Kqueue() {
}


int Kqueue::setEvent(int fd, short ev_filter, u_int fflags) {
	struct kevent register_event;
	EV_SET(&register_event, fd, ev_filter, fflags, 0, 0, NULL);
	changes.push_back(register_event);
	if (kevent(kq, &register_event, 1, NULL, 0, NULL) == -1) {
		std::cout << errno << std::endl;
		perror("kevent Error(register)");
        return -1;
    }
    return 0;
}

int Kqueue::disableEvent(int fd, short ev_filter) {
	struct kevent register_event;
	EV_SET(&register_event, fd, ev_filter, EV_DELETE, 0, 0, NULL);
	changes.push_back(register_event);
	if (kevent(kq, &register_event, 1, NULL, 0, NULL) == -1) {
		perror("kevent Error(in disable)");
        return -1;
    }
	if (ev_filter == EVFILT_WRITE) {
		std::cout << "close in disableEvent" << std::endl;
//		close(fd);
	}
    return 0;
}

int Kqueue::getKq() {
	return kq;
}

int Kqueue::getEventsNum() {
	int event_num = kevent(kq, NULL, 0, reciver_event, 100, &time_over);
	std::cout << "event_num: " << event_num << std::endl;
	return event_num;
}

struct kevent* Kqueue::getReciverEvent() {
	return reciver_event;
}
