#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include <iostream>
#include <map>
#include <vector>
#include <fcntl.h>
//#include "Kqueue.hpp"
#include "Epoll.hpp"
#include "Client.hpp"
#include "Socket.hpp"
#include "Logger.hpp"

#define VecVServ std::vector<VirtualServer>

class EventLoop {
	public:
		EventLoop();
//		EventLoop(Kqueue& kq, std::map<int, std::vector<VirtualServer> >& fd_config_map, std::map<int, std::vector<VirtualServer> >& acceptfd_to_config, std::map<int, Client>& fd_client_map, time_t last_check);
		EventLoop(Epoll& ep, std::map<int, std::vector<VirtualServer> >& fd_config_map, std::map<int, std::vector<VirtualServer> >& acceptfd_to_config, std::map<int, Client>& fd_client_map, time_t last_check);
		EventLoop(const EventLoop& src);
		EventLoop& operator=(const EventLoop& rhs);
		~EventLoop();

		void monitoringEvents();
	private:
//		Kqueue kq;
		Epoll ep;
		time_t last_check;

		std::map<int, std::vector<VirtualServer> >	fd_config_map;
		std::map<int, std::vector<VirtualServer> >	acceptfd_to_config;
		std::map<int, Client>						fd_client_map;
		Logger logger;


		int		handleAccept(int event_fd);
		void	readRequest(int fd, Client& client);
		void	sendResponse(int acceptfd);
		void	checkRequestTimeOut();
		void	sendTimeOutResponse(int fd);
		void	closeConnection(int fd);
};

#endif
