#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "conf/virtualServer.hpp"
#include "conf/Location.hpp"
#include "http/httpReq.hpp"
#include "http/HttpRes.hpp"

class HttpRes;

class Client {
	private:
		httpReq httpreq;
		HttpRes httpres;

		virtualServer vServer;
		int fd;
//		std::map<std::string, Location> uritolocation;
        std::string client_ip;
        int port;
	public:
		Client();
		~Client();
		Client(const Client& source);
		Client& operator =(const Client& source);

		void setFd(int fd);

		void setHttpReq(const HttpRes& source);
        void setHttpReq(httpReq httpreq);
		void setHttpRes(HttpRes source);
		void setVserver(const virtualServer& source);
//		void setUritolocation(const std::map<std::string, Location> map);
        void setClientIp(std::string client_ip);
        void setPort(int port);

		int getFd() const;
		httpReq getHttpReq() const;
		HttpRes getHttpRes() const;
        HttpRes* getHttpResp() const;
		virtualServer getVserver() const;
//		std::map<std::string, Location> getUriToLocation() const;
        std::string getClientIp() const;
        int getPort() const;
};

#endif
