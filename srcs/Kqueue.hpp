#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <iostream>
#include <vector>
#include "Logger.hpp"

class Kqueue {
	private:
		std::vector<struct kevent> changes;
		struct kevent reciver_event[100];
		int kq;
		struct timespec time_over;
		Logger logger;
	public:
		Kqueue();
        Kqueue(const Kqueue& src);
        Kqueue& operator=(const Kqueue& rhs);
		~Kqueue();

		struct kevent* getReciverEvent();
		int getEventsNum();
		int getKq();
		int setEvent(int fd, short ev_filter, u_int fflags);
};

#endif
