#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <iostream>
#include <vector>
#include <unistd.h>

class Kqueue {
	private:
		// 仮置き10
		//struct kevent register_event[10], reciver_event[10];
		std::vector<struct kevent> changes;
		//struct kevent *reciver_event;
		struct kevent reciver_event[100];
		int kq;
		struct timespec time_over;
	public:
		Kqueue();
        Kqueue(const Kqueue& src);
        Kqueue& operator=(const Kqueue& rhs);
		~Kqueue();
		struct kevent* get_reciver_event();
		int get_events_num();
		int get_kq();
		void set_event(int fd, short ev_filter);
        void disable_event(int fd, short ev_filter);
//		void set_register_event(int fd);
};

#endif
