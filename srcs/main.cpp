#include <iostream>
#include <map>
#include <set>
#include <utility>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include "Socket.hpp"
#include "Logger.hpp"
//#include "Kqueue.hpp"
#include "Epoll.hpp"
#include "Client.hpp"
#include "conf/ConfigParser.hpp"
#include "http/HttpReq.hpp"
#include "EventLoop.hpp"

//void initializeFd(ConfigParser conf, Kqueue &kqueue, std::map<int, std::vector<VirtualServer> >& fd_config_map) {
void initializeFd(ConfigParser conf, Epoll& ep, std::map<int, std::vector<VirtualServer> >& fd_config_map) {
	std::vector<VirtualServer> server_confs = conf.getServerConfs();
	std::map<int, int> m;
	for (std::vector<VirtualServer>::iterator it = server_confs.begin(); it != server_confs.end(); it++) {
        std::vector<int> listen = it->getListen();
        for (std::vector<int>::iterator it_listen = listen.begin(); it_listen != listen.end(); ++it_listen) {

            Socket socket;
            if (m.find(*it_listen) == m.end()) {
                socket = Socket(*it_listen);

                if (socket.setSocket() != 0) {
                    std::exit(1);
                }
                m[*it_listen] = socket.getListenFd();
//                if (kqueue.setEvent(socket.getListenFd(), EVFILT_READ, EV_ADD | EV_ENABLE) != 0) {
                if (ep.setEvent(socket.getListenFd(), EPOLLIN, EPOLL_CTL_ADD) != 0) {
                    std::exit(1);
                }

            }
            fd_config_map[m[*it_listen]].push_back(*it);
        }
	}
}

ConfigParser handleConfig(int argc, char *argv[]) {
	if (argc != 1 && argc != 2) {
		std::cerr << "usage: ./webserv *(path_to_config_file)" << std::endl;
		std::exit(1);
	}
	std::string config_path = (argc == 1? "conf/valid_test/tmp.conf": argv[1]);
    if (access(config_path.c_str(), R_OK) != 0) {
        std::cerr << "couldn't open the specified config file" << std::endl;
        std::exit(1);
    }
	ConfigParser conf;
	try {
		std::string txt= readConfFile(config_path);
		conf.setBuf(txt);
		conf.parseConf();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		std::exit(1);
	}
	return conf;
}

int main(int argc, char *argv[]) {
	std::map<int, std::vector<VirtualServer> > fd_config_map;
	std::map<int, std::vector<VirtualServer> > acceptfd_to_config;
	std::map<int, Client> fd_client_map;
//	Kqueue kqueue;
	Epoll ep;

	ConfigParser conf = handleConfig(argc, argv);

//	initializeFd(conf, kqueue, fd_config_map);
	initializeFd(conf, ep, fd_config_map);

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		std::cerr << "signal faild" << std::endl;
		std::exit(1);
	}
	time_t last_check = std::time(0);

//	EventLoop ev_loop(kqueue, fd_config_map, acceptfd_to_config, fd_client_map, last_check);
	EventLoop ev_loop(ep, fd_config_map, acceptfd_to_config, fd_client_map, last_check);
	ev_loop.monitoringEvents();
}
