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
#include "EventLoop.hpp"
#include <map>
#include <set>
#include <utility>

void initializeFd(configParser conf, Kqueue &kqueue, std::map<int, std::vector<virtualServer> >& fd_config_map) {
	std::vector<virtualServer> server_confs = conf.getServerConfs();
	std::map<int, int> m;
	for (std::vector<virtualServer>::iterator it = server_confs.begin(); it != server_confs.end(); it++) {
        std::vector<int> listen = it->getListen();
        for (std::vector<int>::iterator it_listen = listen.begin(); it_listen != listen.end(); ++it_listen) {

            Socket socket;
            if (m.find(*it_listen) == m.end()) {
                std::cout << "listen: " << *it_listen << std::endl;
                socket = Socket(*it_listen);

                if (socket.setSocket() != 0) {
                    std::exit(1);
                }
                m[*it_listen] = socket.getListenFd();
                if (kqueue.setEvent(socket.getListenFd(), EVFILT_READ, EV_ADD | EV_ENABLE) != 0) {
                    std::exit(1);
                }

            }
            fd_config_map[m[*it_listen]].push_back(*it);
            std::cout << "size: " << fd_config_map[m[*it_listen]].size() << std::endl;
        }
	}
}

configParser handleConfig(int argc, char *argv[]) {
	if (argc != 1 && argc != 2) {
		std::cout << "usage: ./webserv *(path_to_config_file)" << std::endl;
		exit(1);
		//return 1;
	}
	std::string config_path = (argc == 1? "conf/valid_test/tmp.conf": argv[1]);
    if (access(config_path.c_str(), R_OK) != 0) {
        std::cerr << "couldn't open the specified config file" << std::endl;
        exit(1);
        //return 1;
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
	return conf;
}

int main(int argc, char *argv[]) {
	std::map<int, std::vector<virtualServer> > fd_config_map;
	std::map<int, std::vector<virtualServer> > acceptfd_to_config;
	std::map<int, Client> fd_client_map;
	Kqueue kqueue;

	configParser conf = handleConfig(argc, argv);

	initializeFd(conf, kqueue, fd_config_map);
	std::cout << fd_config_map.size() << std::endl;

	time_t last_check = std::time(0);

	EventLoop ev_loop(kqueue, fd_config_map, acceptfd_to_config, fd_client_map, last_check);
	ev_loop.monitoringEvents();
}
