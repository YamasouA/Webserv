#include "Socket.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include "Logger.hpp"
#include "Kqueue.hpp"
#include "Client.hpp"
#include "conf/configParser.hpp"
#include "http/httpReq.hpp"
#include <map>
#include <set>
#include <utility>

void sendResponse(int acceptfd, Kqueue &kq, std::map<int, Client> &fd_client_map) {
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
		if (send_cnt < 0)
			return;
		res.setIsSendedHeader(true);
	    client.setHttpRes(res);
		fd_client_map[acceptfd] = client;
//		kq.setEvent(acceptfd, EVFILT_READ, EV_ENABLE);
		return;
	}
	std::cout << "=== send body ===" << std::endl;
//	if (res.getBodySize() > 0) {
//	fcntl(acceptfd, F_SETFL, O_NONBLOCK);
	send_cnt = write(acceptfd, res.getResBody().c_str(), res.getBodySize());
	if (send_cnt < 0)
		return;
//	}
	res.setIsSendedBody(true);
	client.setHttpRes(res);
	fd_client_map.erase(acceptfd);
	kq.setEvent(acceptfd, EVFILT_WRITE, EV_DISABLE);
//	kq.disableEvent(acceptfd, EVFILT_WRITE);
//	kq.setEvent(acceptfd, EVFILT_READ, EV_ENABLE);
	std::cout << "=== DONE ===" << std::endl;
	// fdのクローズは多分ここ
}

std::string inet_ntop4(struct in_addr *addr, char *buf, size_t len) {
	std::string ip;
	(void) buf;
	(void) len;
	// 1バイトずつアクセスできるようにする
	const u_int8_t *ap = (const u_int8_t *)&addr->s_addr;
    std::stringstream ss;
    for (size_t i = 0; ap[i] != '\0'; ++i) {
        std::cout << "ap: " << i << "   :" << ap[i] << std::endl;
    }
    std::cout << std::endl;
    ss << ap[0] << "." << ap[1] << "." << ap[2] << "." << ap[3];
    ss >> ip;
	return ip;
}

std::string my_inet_ntop(struct in_addr *addr, char *buf, size_t len) {
	std::string ip;
	(void) buf;
	(void) len;
    char *p = (char *)addr;
    std::stringstream ss;
    ss << (int)p[0] << "." << (int)p[1] << "." << (int)p[2] << "." << (int)p[3]; //もしint系とchar系ごっちゃに出来なかった場合は一つずつap[i]を変換
    ss >> ip;
	return ip;
}

void initializeFd(configParser conf, Kqueue &kqueue, std::map<int, std::vector<virtualServer> >& fd_config_map) {
	std::vector<virtualServer> server_confs = conf.getServerConfs();
	std::map<int, int> m;
	for (std::vector<virtualServer>::iterator it = server_confs.begin(); it != server_confs.end(); it++) {
        std::vector<int> listen = it->getListen();
        for (std::vector<int>::iterator it_listen = listen.begin(); it_listen != listen.end(); ++it_listen) {

            Socket *socket;
            if (m.find(*it_listen) == m.end()) {
                std::cout << "listen: " << *it_listen << std::endl;
                socket = new Socket(*it_listen);

                if (socket->setSocket() != 0) {
                    std::exit(1);
                }
                m[*it_listen] = socket->getListenFd();
                if (kqueue.setEvent(socket->getListenFd(), EVFILT_READ, EV_ADD | EV_ENABLE) != 0) {
                    std::exit(1);
                }

            }
            fd_config_map[m[*it_listen]].push_back(*it);
            std::cout << "size: " << fd_config_map[m[*it_listen]].size() << std::endl;
        }
	}
}

void assignServer(std::vector<virtualServer> server_confs, Client& client) {
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
	client.setVserver(server_confs[0]);
}

void readRequest(int fd, Client& client, std::vector<virtualServer> server_confs, Kqueue kq) {
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	ssize_t recv_cnt = 0;
	std::cout << "read_request" << std::endl;
	httpReq httpreq = client.getHttpReq();

	fcntl(fd, F_SETFL, O_NONBLOCK);
	recv_cnt = recv(fd, buf, sizeof(buf) - 1, 0);
//	if (recv_cnt == 0) {
//		httpreq.setIsReqEnd();
//	}
	if (recv_cnt < 0) {
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

	client.setFd(fd);
    client.setHttpReq(httpreq);
    assignServer(server_confs, client);
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

int main(int argc, char *argv[]) {
	// 設定ファイルを読み込む
	// read_config();
	(void)argc;
	(void)argv;
	std::map<int, std::vector<virtualServer> > fd_config_map;
	std::map<int, std::vector<virtualServer> > acceptfd_to_config;
	std::map<int, Client> fd_client_map;

	if (argc != 1 && argc != 2) {
		std::cout << "usage: ./webserv *(path_to_config_file)" << std::endl;
		return 1;
	}
	std::string config_path = (argc == 1? "conf/valid_test/tmp.conf": argv[1]);
    if (access(config_path.c_str(), R_OK) != 0) {
        std::cerr << "couldn't open the specified config file" << std::endl;
        return 1;
    }
	configParser conf;
	try {
		std::string txt= readConfFile(config_path);
		conf.set_buf(txt);
		conf.parseConf();
	} catch (const std::exception &e) {
		std::cout << e.what() << std::endl;
		std::exit(1);
	}
	Kqueue kqueue;
	initializeFd(conf, kqueue, fd_config_map);
	std::cout << fd_config_map.size() << std::endl;
	int acceptfd;

	struct timespec time_over;
	time_over.tv_sec = 10;
	time_over.tv_nsec = 0;

	while (1) {
		int events_num = kqueue.getEventsNum();
		if (events_num == -1) {
			perror("kevent");
			std::exit(1);
		} else if (events_num == 0) {
			std::cout << "time over" << std::endl;
			continue;
		}

		for (int i = 0; i < events_num; ++i) {
			struct kevent* reciver_event = kqueue.getReciverEvent();
			int event_fd = reciver_event[i].ident;
//			fcntl(event_fd, F_SETFL, O_NONBLOCK);
			std::cout << "event_fd(): " << event_fd << std::endl;
			if (reciver_event[i].flags & EV_EOF) {
				std::cout << "Client " << event_fd << " has disconnected" << std::endl;
				close(event_fd);
			} else if (fd_config_map.count(event_fd) == 1) {
				Client client;
                struct sockaddr_in client_addr;
                socklen_t sock_len = sizeof(client_addr);
				// ここのevent_fdはconfigで設定されてるserverのfd
				acceptfd = accept(event_fd, (struct sockaddr *)&client_addr, &sock_len);
				fcntl(acceptfd, F_SETFL, O_NONBLOCK);
				acceptfd_to_config[acceptfd] = fd_config_map[event_fd];
				if (acceptfd == -1) {
					std::cerr << "Accept socket Error" << std::endl;
					continue;
				}
                std::string client_ip = my_inet_ntop(&(client_addr.sin_addr), NULL, 0);
                client.setClientIp(client_ip); // or Have the one after adapting inet_ntoa
                struct sockaddr_in sin;
                socklen_t addrlen = sizeof(sin);
                getsockname(event_fd, (struct sockaddr *)&sin, &addrlen);
                int port_num = ntohs(sin.sin_port);
                client.setPort(port_num);
				fd_client_map[acceptfd] =  client;
				kqueue.setEvent(acceptfd, EVFILT_READ, EV_ADD | EV_ENABLE);
				kqueue.setEvent(acceptfd, EVFILT_WRITE, EV_ADD | EV_DISABLE);
			} else if (reciver_event[i].filter ==  EVFILT_READ) {
                std::cout << "==================READ_EVENT==================" << std::endl;
//				acceptfd = event_fd;
				char buf[1024];
				memset(buf, 0, sizeof(buf));
				readRequest(event_fd, fd_client_map[event_fd], acceptfd_to_config[event_fd], kqueue);
			} else if (reciver_event[i].filter == EVFILT_WRITE) {
				std::cout << "==================WRITE_EVENT==================" << std::endl;
//				acceptfd = event_fd;
                HttpRes res = fd_client_map[event_fd].getHttpRes();
				sendResponse(event_fd, kqueue, fd_client_map);
			}
		}
	}
}
