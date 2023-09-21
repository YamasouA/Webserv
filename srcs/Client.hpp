#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "conf/VirtualServer.hpp"
#include "conf/Location.hpp"
#include "http/HttpReq.hpp"
#include "http/HttpRes.hpp"

class HttpRes;

class Client {
	private:
		HttpReq httpreq;
		HttpRes httpres;

		VirtualServer vServer;
		int fd;
        std::string client_ip;
        int port;
		time_t last_recv_time;
		bool is_req_end;
	public:
		Client();
		~Client();
		Client(const Client& source);
		Client& operator =(const Client& source);

		void setFd(int fd);

		void setHttpReq(const HttpRes& source);
        void setHttpReq(HttpReq httpreq);
		void setHttpRes(HttpRes source);
		void setVserver(const VirtualServer& source);
        void setClientIp(std::string client_ip);
        void setPort(int port);
		void setLastRecvTime(time_t now);
		void setEndOfReq(bool flag);
		bool isEndOfReq() const;

		int getFd() const;
		HttpReq getHttpReq() const;
		HttpRes getHttpRes() const;
        HttpRes* getHttpResp() const;
		VirtualServer getVserver() const;
		time_t getLastRecvTime() const;
        std::string getClientIp() const;
        int getPort() const;
};

#endif
