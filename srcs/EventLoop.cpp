#include "EventLoop.hpp"

EventLoop::EventLoop()
{}

EventLoop::EventLoop(Epoll& ep, std::map<int, std::vector<VirtualServer> >& fd_config_map, std::map<int, std::vector<VirtualServer> >& acceptfd_to_config, std::map<int, Client>& fd_client_map, time_t last_check)
:ep(ep),
	last_check(last_check)
{
	this->fd_config_map = fd_config_map;
	this->acceptfd_to_config = acceptfd_to_config;
	this->fd_client_map = fd_client_map;
}

EventLoop::EventLoop(const EventLoop& src)
:ep(src.ep),
	last_check(src.last_check)
{
	this->fd_config_map = src.fd_config_map;
	this->acceptfd_to_config = src.acceptfd_to_config;
	this->fd_client_map = src.fd_client_map;
}

EventLoop& EventLoop::operator=(const EventLoop& rhs) {
	if (this == &rhs) {
		return *this;
	}
	return *this;
}

EventLoop::~EventLoop() {

}

void EventLoop::closeConnection(int fd) {
	logger.logging("close connection");
	acceptfd_to_config.erase(fd);
	fd_client_map.erase(fd);
	close(fd);
}

void EventLoop::sendResponse(int acceptfd) {
	fcntl(acceptfd, F_SETFL, O_NONBLOCK);
	ssize_t send_cnt;
	Client client = fd_client_map[acceptfd];
	HttpRes res = client.getHttpRes();

	if (!res.getIsSendedHeader()) {
		logger.logging("=== HTTP Response Header ===");
		logger.logging(res.getBuf());
		send_cnt = write(acceptfd, res.getBuf().c_str(), res.getHeaderSize());
		if (send_cnt < 0 || send_cnt == 0) {
			return closeConnection(acceptfd);
		}
		res.setIsSendedHeader(true);
	    client.setHttpRes(res);
		fd_client_map[acceptfd] = client;
		if (res.isHeaderOnly() && !res.getKeepAlive()) {
			return closeConnection(acceptfd);
		}
		return;
	}
	if (res.getBodySize() > 0) {
		logger.logging("=== HTTP Response Body===");
		logger.logging(res.getResBody());
		send_cnt = write(acceptfd, res.getResBody().c_str(), res.getBodySize());
		if (send_cnt < 0 || send_cnt == 0) {
			return closeConnection(acceptfd);
		}
	}
	res.setIsSendedBody(true);
	client.setHttpRes(res);
	if (!res.getKeepAlive()) {
		return closeConnection(acceptfd);
	}
	ep.setEvent(acceptfd, EPOLLIN, EPOLL_CTL_MOD);
	client.setIsSendRes(true);
	client.setLastConnectTime(std::time(0));
	HttpReq tmp = HttpReq();
	client.setHttpReq(tmp);
	fd_client_map[acceptfd] = client;
}

void EventLoop::sendTimeOutResponse(int fd) {
	Client client = fd_client_map[fd];
	HttpReq req;
    req.setClientIP(client.getClientIp());
    req.setPort(client.getPort());
	client.setHttpReq(req);
	HttpRes res(client);
	res.handleReqErr(408);
	client.setHttpRes(res);
	fd_client_map[fd] = client;
	ep.setEvent(fd, EPOLLOUT, EPOLL_CTL_MOD);
}

static std::string my_inet_ntop(struct in_addr *addr, char *buf, size_t len) {
	std::string ip;
	(void) buf;
	(void) len;
    char *p = (char *)addr;
    std::stringstream ss;
    ss << (int)p[0] << "." << (int)p[1] << "." << (int)p[2] << "." << (int)p[3];
    ss >> ip;
	return ip;
}

static void assignServer(std::vector<VirtualServer> server_confs, Client& client) {
	for (std::vector<VirtualServer>::iterator it = server_confs.begin();
		it != server_confs.end(); it++) {

        std::map<std::string, std::string> tmp = client.getHttpReq().getHeaderFields();
        std::string host_name_and_port;
        std::string host_name;
		host_name_and_port = client.getHttpReq().getHeaderFields()["host"];
		std::string::size_type pos = host_name_and_port.find(':');
		if (pos != std::string::npos) {
			host_name = host_name_and_port.substr(0, pos);
		}
        std::vector<std::string> vec = it->getServerNames();
        for (std::vector<std::string>::iterator vit = vec.begin(); vit != vec.end(); ++vit) {
			if (*vit == host_name_and_port) {
				client.setVserver(*it);
				return;
			}
			else if (*vit == host_name) {
                client.setVserver(*it);
                return;
            }
        }
	}
	client.setVserver(server_confs[0]);
}

void EventLoop::readRequest(int fd, Client& client) {
	char buf[1024];
	std::memset(buf, 0, sizeof(buf));
	ssize_t recv_cnt = 0;
	HttpReq httpreq = client.getHttpReq();
	client.setIsSendRes(false);

	recv_cnt = recv(fd, buf, sizeof(buf) - 1, 0);
	client.setLastRecvTime(std::time(0));
	if (recv_cnt < 0 || recv_cnt == 0) {
		return closeConnection(fd);
	} else {
		buf[recv_cnt] = '\0';
		httpreq.appendReq(buf);
		if (httpreq.isEndOfHeader() && httpreq.getHeaderFields().size() == 0) {
			httpreq.parseHeader();
		}
		if ((httpreq.isEndOfHeader() && httpreq.getHeaderFields().size() != 0)) {
			httpreq.parseBody();
		}
	}
	if (!httpreq.isEndOfReq()) {
		client.setHttpReq(httpreq);
		client.setEndOfReq(false);
		return;
    }
	logger.logging("=== HTTP Request ===");
	logger.logging(httpreq.getBuf());
	client.setEndOfReq(true);
    httpreq.setClientIP(client.getClientIp());
    httpreq.setPort(client.getPort());

    client.setHttpReq(httpreq);
    assignServer(acceptfd_to_config[fd], client);
    HttpRes respons(client);
    if (httpreq.getErrStatus() > 0) {
        respons.handleReqErr(httpreq.getErrStatus());
    } else {
        respons.runHandlers();
    }

    client.setHttpRes(respons);
	ep.setEvent(fd, EPOLLOUT, EPOLL_CTL_MOD);
}

void EventLoop::checkRequestTimeOut() {
	const int time_out = 8;
	const int time_check_span = 3;
	time_t now;

	now = std::time(0);
	if (now - last_check > time_check_span) {
		std::map<int, Client>::iterator it = fd_client_map.begin();
		while(it != fd_client_map.end()) {
			if (now - it->second.getLastRecvTime() > time_out && it->second.isEndOfReq() == false) {
				int fd = it->second.getFd();
				it++;
				sendTimeOutResponse(fd);
			} else if (now - it->second.getLastConnectTime() > time_out && it->second.isSendRes() == true) {
				int fd = it->second.getFd();
				it++;
				closeConnection(fd);
			} else {
				it++;
			}
		}
		last_check = now;
	}
}

int EventLoop::handleAccept(int event_fd) {
	Client client;
	struct sockaddr_in client_addr;
	socklen_t sock_len = sizeof(client_addr);
	// ここのevent_fdはconfigで設定されてるserverのfd
	int acceptfd = accept(event_fd, (struct sockaddr *)&client_addr, &sock_len);
	fcntl(acceptfd, F_SETFL, O_NONBLOCK);
	acceptfd_to_config[acceptfd] = fd_config_map[event_fd];
	if (acceptfd == -1) {
		logger.logging("Accept socket Error");
		return 1;
	}
	std::string client_ip = my_inet_ntop(&(client_addr.sin_addr), NULL, 0);
	client.setClientIp(client_ip); // or Have the one after adapting inet_ntoa
	struct sockaddr_in sin;
	socklen_t addrlen = sizeof(sin);
	getsockname(event_fd, (struct sockaddr *)&sin, &addrlen);
	int port_num = ntohs(sin.sin_port);
	client.setPort(port_num);
	client.setFd(acceptfd);
	fd_client_map[acceptfd] =  client;
	ep.setEvent(acceptfd, EPOLLIN | EPOLLRDHUP, EPOLL_CTL_ADD);
	return 0;
}

void EventLoop::monitoringEvents() {
	while (1) {
		checkRequestTimeOut();
		int events_num = ep.getEventsNum();
		if (events_num == -1) {
			logger.logging("Events num: -1");
			std::exit(1);
		} else if (events_num == 0) {
			continue;
		}

		for (int i = 0; i < events_num; ++i) {
			struct epoll_event* reciver_event = ep.getReciverEvent();
			int event_fd = reciver_event[i].data.fd;
			if (reciver_event[i].events & EPOLLERR) {
				closeConnection(event_fd);
			} else if (fd_config_map.count(event_fd) == 1) {
				if (handleAccept(event_fd))
					continue;
			} else if (reciver_event[i].events ==  EPOLLIN) {
				readRequest(event_fd, fd_client_map[event_fd]);
			} else if (reciver_event[i].events == EPOLLOUT) {
                HttpRes res = fd_client_map[event_fd].getHttpRes();
				sendResponse(event_fd);
			} else if (reciver_event[i].events & (EPOLLRDHUP | EPOLLHUP)) {
				closeConnection(event_fd);
			}
		}
	}
}
