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

void send_response(int acceptfd, Kqueue &kq, std::map<int, Client> &fd_client_map) {
	fcntl(acceptfd, F_SETFL, O_NONBLOCK);
	size_t send_cnt;
	std::cout << "===== send response =====" << std::endl;
	Client client = fd_client_map[acceptfd];
	HttpRes res = client.get_httpRes();

	if (!res.get_is_sended_header()) {
		std::cout << "=== send header ===" << std::endl;
		send_cnt = write(acceptfd, res.buf.c_str(), res.header_size);
		if (send_cnt < 0)
			return;
		res.set_is_sended_header(true);
	    client.set_httpRes(res);
	}
	std::cout << "=== send body ===" << std::endl;
	send_cnt = write(acceptfd, res.out_buf.c_str(), res.body_size);
	if (send_cnt < 0)
		return;
	res.set_is_sended_body(true);
	client.set_httpRes(res);
	kq.disable_event(acceptfd, EVFILT_WRITE);
	fd_client_map.erase(acceptfd);
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

void initialize_fd(configParser conf, Kqueue &kqueue, std::map<int, std::vector<virtualServer> >& fd_config_map) {
	std::vector<virtualServer> server_confs = conf.get_serve_confs();
	std::map<int, int> m;
	for (std::vector<virtualServer>::iterator it = server_confs.begin(); it != server_confs.end(); it++) {
        std::vector<int> listen = it->get_listen();
        for (std::vector<int>::iterator it_listen = listen.begin(); it_listen != listen.end(); ++it_listen) {

            Socket *socket;
            if (m.find(*it_listen) == m.end()) {
                std::cout << "listen: " << *it_listen << std::endl;
                socket = new Socket(*it_listen);
                socket->set_socket();
                m[*it_listen] = socket->get_listenfd();
                kqueue.set_event(socket->get_listenfd(), EVFILT_READ);
            }
            fd_config_map[m[*it_listen]].push_back(*it);
            std::cout << "size: " << fd_config_map[m[*it_listen]].size() << std::endl;
        }
	}
}

void assign_server(std::vector<virtualServer> server_confs, Client& client) {
	for (std::vector<virtualServer>::iterator it = server_confs.begin();
		it != server_confs.end(); it++) {

        std::map<std::string, std::string> tmp = client.get_httpReq().getHeaderFields();
        std::string host_name;
		host_name = client.get_httpReq().getHeaderFields()["host"];
		std::cout << "host name: " << host_name << std::endl;
        std::vector<std::string> vec = it->get_server_names();
        for (std::vector<std::string>::iterator vit = vec.begin(); vit != vec.end(); ++vit) {
            std::cout << "name: " << *vit << std::endl;
            if (*vit == host_name) {
                std::cout << "match name" << std::endl;
                client.set_vServer(*it);
                return;
            }
        }
	}
	std::cout << "no match" << std::endl;
	client.set_vServer(server_confs[0]);
}

void read_request(int fd, Client& client, std::vector<virtualServer> server_confs, Kqueue kq) {
	char buf[1024];

	memset(buf, 0, sizeof(buf));
	fcntl(fd, F_SETFL, O_NONBLOCK);
	size_t recv_cnt = recv(fd, buf, sizeof(buf) - 1, 0);

	if (recv_cnt < 0) {
		return;
	}
	buf[recv_cnt] = '\0';
	httpReq httpreq = client.get_httpReq();
	httpreq.appendReq(buf);
	client.set_httpReq(httpreq);
	if (recv_cnt == sizeof(buf) - 1) {
		return;
    }

    httpreq.setClientIP(client.get_client_ip());
    httpreq.setPort(client.get_port());
	try {
		httpreq.parseRequest();
	} catch (const std::exception &e) {
		std::cout << e.what() << std::endl;
	}
	client.set_fd(fd);
    client.set_httpReq(httpreq);
    assign_server(server_confs, client);
    HttpRes respons(client, kq);
    if (httpreq.getErrStatus() > 0) {
        respons.handleReqErr(httpreq.getErrStatus());
    } else {
        respons.runHandlers();
    }
    client.set_httpRes(respons);
    kq.disable_event(fd, EVFILT_READ);
	kq.set_event(fd, EVFILT_WRITE);
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
	initialize_fd(conf, kqueue, fd_config_map);
	std::cout << fd_config_map.size() << std::endl;
	int acceptfd;

	struct timespec time_over;
	time_over.tv_sec = 10;
	time_over.tv_nsec = 0;

	while (1) {
		int events_num = kqueue.get_events_num();
		if (events_num == -1) {
			perror("kevent");
			std::exit(1);
		} else if (events_num == 0) {
			std::cout << "time over" << std::endl;
			continue;
		}

		for (int i = 0; i < events_num; ++i) {
			struct kevent* reciver_event = kqueue.get_reciver_event();
			int event_fd = reciver_event[i].ident;
			fcntl(event_fd, F_SETFL, O_NONBLOCK);
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
				acceptfd_to_config[acceptfd] = fd_config_map[event_fd];
				if (acceptfd == -1) {
					std::cerr << "Accept socket Error" << std::endl;
					continue;
				}
                std::string client_ip = my_inet_ntop(&(client_addr.sin_addr), NULL, 0);
                client.set_client_ip(client_ip); // or Have the one after adapting inet_ntoa
                struct sockaddr_in sin;
                socklen_t addrlen = sizeof(sin);
                getsockname(event_fd, (struct sockaddr *)&sin, &addrlen);
                int port_num = ntohs(sin.sin_port);
				std::cout << port_num << std::endl;
                client.set_port(port_num);
				fd_client_map[acceptfd] =  client;
				kqueue.set_event(acceptfd, EVFILT_READ);
			} else if (reciver_event[i].filter ==  EVFILT_READ) {
                std::cout << "==================READ_EVENT==================" << std::endl;
				acceptfd = event_fd;
				char buf[1024];
				memset(buf, 0, sizeof(buf));
				std::cout << acceptfd << std::endl;
				read_request(acceptfd, fd_client_map[acceptfd], acceptfd_to_config[acceptfd], kqueue);
			} else if (reciver_event[i].filter == EVFILT_WRITE) {
				std::cout << "==================WRITE_EVENT==================" << std::endl;
				acceptfd = event_fd;
                HttpRes res = fd_client_map[acceptfd].get_httpRes();
				send_response(acceptfd, kqueue, fd_client_map);
			}
		}
	}
}
