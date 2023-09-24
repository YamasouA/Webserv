#include "Kqueue.hpp"

Kqueue::Kqueue()
{
	kq = kqueue();
	if (kq < -1) {
		logger.logging("kqueue Error");
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
    return 0;
}

int Kqueue::getKq() {
	return kq;
}

int Kqueue::getEventsNum() {
	struct kevent *register_event = new struct kevent[changes.size()];
	for (size_t i = 0; i < changes.size(); ++i) {
		register_event[i] = changes[i];
	}
	int event_num = kevent(kq, register_event, changes.size(), reciver_event, 100, &time_over);
	changes.clear();
	delete [] register_event;
	return event_num;
}

struct kevent* Kqueue::getReciverEvent() {
	return reciver_event;
}
