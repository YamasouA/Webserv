#include "EventLoop.hpp"

EventLoop::EventLoop()
{}

EventLoop::EventLoop(Kqueue& kq, std::map<int, std::vector<virtualServer> >& fd_config_map, std::map<int, std::vector<virtualServer> >& acceptfd_to_config, std::map<int, Client>& fd_client_map, time_t last_check)
:kq(kq),
	last_check(last_check)
{
	this->fd_config_map = fd_config_map;
	this->acceptfd_to_config = acceptfd_to_config;
	this->fd_client_map = fd_client_map;
}

EventLoop::EventLoop(const EventLoop& src)
:kq(src.kq),
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


void EventLoop::sendResponse(int acceptfd) {
	fcntl(acceptfd, F_SETFL, O_NONBLOCK);
	size_t send_cnt;
	std::cout << "===== send response =====" << std::endl;
	std::cout << "acceotfd: " << acceptfd << std::endl;
	Client client = fd_client_map[acceptfd];
	HttpRes res = client.getHttpRes();

//	kq.setEvent(acceptfd, EVFILT_READ, EV_ENABLE);
	std::cout << "is sended header: " << res.getIsSendedHeader() << std::endl;
	if (!res.getIsSendedHeader()) {
		std::cout << "=== send header ===" << std::endl;
//		fcntl(acceptfd, F_SETFL, O_NONBLOCK);
		send_cnt = write(acceptfd, res.getBuf().c_str(), res.getHeaderSize());
		if (send_cnt < 0 || send_cnt == 0) {
			acceptfd_to_config.erase(acceptfd);
			fd_client_map.erase(acceptfd);
			close(acceptfd);
			return;
		}
		res.setIsSendedHeader(true);
	    client.setHttpRes(res);
		fd_client_map[acceptfd] = client;
		if (res.isHeaderOnly() && !res.getKeepAlive()) {
			std::cout << "Disconnect client fd: " << acceptfd << std::endl;
//			kq.setEvent(acceptfd, EVFILT_WRITE, EV_DELETE);
//			kq.setEvent(acceptfd, EVFILT_READ, EV_DELETE);
			acceptfd_to_config.erase(acceptfd);
			fd_client_map.erase(acceptfd);
			close(acceptfd);
			return;
		}
//		kq.setEvent(acceptfd, EVFILT_READ, EV_ENABLE);
		return;
	}
	std::cout << "=== send body ===" << std::endl;
//	if (res.getBodySize() > 0) {
//	fcntl(acceptfd, F_SETFL, O_NONBLOCK);
	if (res.getBodySize() > 0) {
		send_cnt = write(acceptfd, res.getResBody().c_str(), res.getBodySize());
		if (send_cnt < 0 || send_cnt == 0) {
			acceptfd_to_config.erase(acceptfd);
			fd_client_map.erase(acceptfd);
			close(acceptfd);
			return;
		}
	}
//	}
	res.setIsSendedBody(true);
	client.setHttpRes(res);
	kq.setEvent(acceptfd, EVFILT_WRITE, EV_DISABLE);
	std::cout << "keep-alive: " << res.getKeepAlive() << std::endl;
	if (!res.getKeepAlive()) {
		std::cout << "Disconnect client fd: " << acceptfd << std::endl;
//		kq.setEvent(acceptfd, EVFILT_WRITE, EV_DELETE);
//		kq.setEvent(acceptfd, EVFILT_READ, EV_DELETE);
		acceptfd_to_config.erase(acceptfd);
		fd_client_map.erase(acceptfd);
		close(acceptfd);
		return;
	}
//	fd_client_map.erase(acceptfd);
	httpReq tmp = httpReq();
	client.setHttpReq(tmp);
	fd_client_map[acceptfd] = client;
//	kq.disableEvent(acceptfd, EVFILT_WRITE);
//	kq.setEvent(acceptfd, EVFILT_READ, EV_ENABLE);
	std::cout << "=== DONE ===" << std::endl;
	// fdのクローズは多分ここ
}

void EventLoop::sendTimeOutResponse(int fd) {
	Client client = fd_client_map[fd];
	httpReq req;
    req.setClientIP(client.getClientIp());
    req.setPort(client.getPort());
	client.setHttpReq(req);
//	HttpRes res = client.getHttpRes();
	HttpRes res(client, kq);
	res.handleReqErr(408);
	client.setHttpRes(res);
	fd_client_map[fd] = client;
	kq.setEvent(fd, EVFILT_WRITE, EV_ADD | EV_ENABLE);
//	sendResponse(fd, kq, fd_client_map);
//	res.createErrorResponse(408);
	// なぜかtrueが入っている
//	res.setIsSendedHeader(false);
//	client.setHttpRes(res);
//	fd_client_map[fd] = client;
//	std::cout << res.getIsSendedHeader() << std::endl;
//	sendResponse(fd, kq, fd_client_map);
}

//static std::string inet_ntop4(struct in_addr *addr, char *buf, size_t len) {
//	std::string ip;
//	(void) buf;
//	(void) len;
//	// 1バイトずつアクセスできるようにする
//	const u_int8_t *ap = (const u_int8_t *)&addr->s_addr;
//    std::stringstream ss;
//    for (size_t i = 0; ap[i] != '\0'; ++i) {
//        std::cout << "ap: " << i << "   :" << ap[i] << std::endl;
//    }
//    std::cout << std::endl;
//    ss << ap[0] << "." << ap[1] << "." << ap[2] << "." << ap[3];
//    ss >> ip;
//	return ip;
//}

static std::string my_inet_ntop(struct in_addr *addr, char *buf, size_t len) {
	std::string ip;
	(void) buf;
	(void) len;
    char *p = (char *)addr;
    std::stringstream ss;
    ss << (int)p[0] << "." << (int)p[1] << "." << (int)p[2] << "." << (int)p[3]; //もしint系とchar系ごっちゃに出来なかった場合は一つずつap[i]を変換
    ss >> ip;
	return ip;
}

static void assignServer(std::vector<virtualServer> server_confs, Client& client) {
	for (std::vector<virtualServer>::iterator it = server_confs.begin();
		it != server_confs.end(); it++) {

        std::map<std::string, std::string> tmp = client.getHttpReq().getHeaderFields();
        std::string host_name;
		host_name = client.getHttpReq().getHeaderFields()["host"];
		std::cout << "host name: " << host_name << std::endl;
        std::vector<std::string> vec = it->getServerNames();
        for (std::vector<std::string>::iterator vit = vec.begin(); vit != vec.end(); ++vit) {
            std::cout << "name: " << *vit << std::endl;
            if (*vit == host_name) {
                std::cout << "match name" << std::endl;
                client.setVserver(*it);
                return;
            }
        }
	}
	std::cout << "no match" << std::endl;
	std::cout << "ok" << std::endl;
	client.setVserver(server_confs[0]);
	std::cout << "ok" << std::endl;
}

//void readRequest(int fd, Client& client, std::vector<virtualServer> server_confs, Kqueue kq) {
void EventLoop::readRequest(int fd, Client& client) {
	char buf[1024];
	std::memset(buf, 0, sizeof(buf));
	ssize_t recv_cnt = 0;
	std::cout << "read_request" << std::endl;
	httpReq httpreq = client.getHttpReq();

//	fcntl(fd, F_SETFL, O_NONBLOCK);
	recv_cnt = recv(fd, buf, sizeof(buf) - 1, 0);
//	if (recv_cnt == 0) {
//		httpreq.setIsReqEnd();
//	}
	client.setLastRecvTime(std::time(0));
	if (recv_cnt < 0 || recv_cnt == 0) {
		acceptfd_to_config.erase(fd);
		fd_client_map.erase(fd);
		close(fd);
		std::cout << "ko" << std::endl;
		return;
	} else {
		buf[recv_cnt] = '\0';
		httpreq.appendReq(buf);
		if (httpreq.isEndOfHeader() && httpreq.getHeaderFields().size() == 0) {
			httpreq.parseHeader();
			std::cout << httpreq << std::endl;
		}
		if ((httpreq.isEndOfHeader() && httpreq.getHeaderFields().size() != 0)) {
			httpreq.parseBody();
		}
	}
	if (!httpreq.isEndOfReq()) {
		client.setHttpReq(httpreq);
		return;
    }
    httpreq.setClientIP(client.getClientIp());
    httpreq.setPort(client.getPort());

	//client.setFd(fd);
    client.setHttpReq(httpreq);
    assignServer(acceptfd_to_config[fd], client);
//	acceptfd_to_config.erase(fd);
//    assignServer(server_confs, client);
    HttpRes respons(client, kq);
    if (httpreq.getErrStatus() > 0) {
        respons.handleReqErr(httpreq.getErrStatus());
    } else {
        respons.runHandlers();
    }

    client.setHttpRes(respons);
//	kq.setEvent(fd, EVFILT_READ, EV_DISABLE);
//    kq.disableEvent(fd, EVFILT_READ);
	kq.setEvent(fd, EVFILT_WRITE, EV_ENABLE);
}

void EventLoop::checkRequestTimeOut() {
	const int time_out = 1;
	const int time_check_span = 3;
	time_t now;

	now = std::time(0);
	if (now - last_check > time_check_span) {
		std::map<int, Client>::iterator it = fd_client_map.begin();
		while(it != fd_client_map.end()) {
			std::cout << "hoge" << std::endl;
			std::cout << "fd: " << it->second.getFd() << std::endl;
			if (now - it->second.getLastRecvTime() > time_out) {
				int fd = it->second.getFd();
				it++;
				sendTimeOutResponse(fd);
				//kqueue.setEvent(fd, EVFILT_WRITE, EV_DELETE);
				//kqueue.setEvent(fd, EVFILT_READ, EV_DELETE);
//				fd_client_map.erase(fd);
//				close(fd);
			} else {
				it++;
			}
		}
		last_check = now;
	}
}

int EventLoop::handleAccept(int event_fd) {
	std::cout << "================handle accept===================" << std::endl;
	Client client;
	struct sockaddr_in client_addr;
	socklen_t sock_len = sizeof(client_addr);
	// ここのevent_fdはconfigで設定されてるserverのfd
	int acceptfd = accept(event_fd, (struct sockaddr *)&client_addr, &sock_len);
	fcntl(acceptfd, F_SETFL, O_NONBLOCK);
	acceptfd_to_config[acceptfd] = fd_config_map[event_fd];
	if (acceptfd == -1) {
		std::cerr << "Accept socket Error" << std::endl;
		return 1;
//		continue;
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
	kq.setEvent(acceptfd, EVFILT_READ, EV_ADD | EV_ENABLE);
	kq.setEvent(acceptfd, EVFILT_WRITE, EV_ADD | EV_DISABLE);
	return 0;
}

void EventLoop::monitoringEvents() {
	while (1) {
		checkRequestTimeOut();
		int events_num = kq.getEventsNum();
		if (events_num == -1) {
			perror("kevent");
			std::exit(1);
		} else if (events_num == 0) {
			std::cout << "time over" << std::endl;
			continue;
		}

		for (int i = 0; i < events_num; ++i) {
			struct kevent* reciver_event = kq.getReciverEvent();
			int event_fd = reciver_event[i].ident;
//			fcntl(event_fd, F_SETFL, O_NONBLOCK);
			std::cout << "event_fd(): " << event_fd << std::endl;
			if (reciver_event[i].flags & EV_EOF) {
				std::cout << "Client " << event_fd << " has disconnected" << std::endl;
//				kqueue.setEvent(event_fd, EVFILT_WRITE, EV_DELETE);
//				kqueue.setEvent(event_fd, EVFILT_READ, EV_DELETE);
				acceptfd_to_config.erase(event_fd);
				fd_client_map.erase(event_fd);
				close(event_fd);
			} else if (fd_config_map.count(event_fd) == 1) {
				if (handleAccept(event_fd))
					continue;
			} else if (reciver_event[i].filter ==  EVFILT_READ) {
                std::cout << "==================READ_EVENT==================" << std::endl;
				char buf[1024];
				std::memset(buf, 0, sizeof(buf));
				readRequest(event_fd, fd_client_map[event_fd]);
//				readRequest(event_fd, fd_client_map[event_fd], acceptfd_to_config[event_fd], kqueue);
			} else if (reciver_event[i].filter == EVFILT_WRITE) {
				std::cout << "==================WRITE_EVENT==================" << std::endl;
                HttpRes res = fd_client_map[event_fd].getHttpRes();
				sendResponse(event_fd);
			}
		}
	}
}
