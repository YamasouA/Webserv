#ifndef EPOLL_HPP
#define EPOLL_HPP

#include <stdio.h>
#include <errno.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/epoll.h>

class Epoll {
	public:
		Epoll();
		Epoll(const Epoll& src);
		Epoll& operator=(const Epoll& rhs);
		~Epoll();

		struct epoll_event* getReciverEvent();
		int getEventsNum();
		int setEvent(int fd, uint32_t event, int op);
	private:
		static const int kTimeout = 1000;
		struct epoll_event reciver_event[10];
		int epfd;

};

#endif
