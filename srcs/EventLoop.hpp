#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include <iostream>
#include <map>
#include <vector>
#include <fcntl.h>
#include "Kqueue.hpp"
#include "Client.hpp"
#include "Socket.hpp"

#define VecVServ std::vector<virtualServer>

class EventLoop {
	public:
		EventLoop();
		EventLoop(Kqueue& kq, std::map<int, std::vector<virtualServer> >& fd_config_map, std::map<int, std::vector<virtualServer> >& acceptfd_to_config, std::map<int, Client>& fd_client_map, time_t last_check);
		EventLoop(const EventLoop& src);
		EventLoop& operator=(const EventLoop& rhs);
		~EventLoop();

		void monitoringEvents();
	private:
		std::map<int, std::vector<virtualServer> > fd_config_map;
		std::map<int, std::vector<virtualServer> > acceptfd_to_config;
		std::map<int, Client> fd_client_map;
		Kqueue kq;
		time_t last_check;
//		configParser conf;

//		void assignServer(std::vector<virtualServer> server_confs, Client& client);
		int handleAccept(int event_fd);
		void readRequest(int fd, Client& client);
		void sendResponse(int acceptfd);
		void checkRequestTimeOut();
		void sendTimeOutResponse(int fd);
};

#endif