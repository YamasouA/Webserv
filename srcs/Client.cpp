#include "Client.hpp"
#include "http/HttpRes.hpp"

Client::Client()
: httpreq(),
fd(0),
last_recv_time(std::time(0))
{}

Client::Client(const Client& source)
:httpreq(source.getHttpReq()),
    httpres(source.getHttpRes()),
    vServer(source.getVserver()),
	fd(source.getFd()),
    client_ip(source.getClientIp()),
    port(source.getPort()),
	last_recv_time(source.getLastRecvTime()),
	is_req_end(source.isEndOfReq())
{}

Client& Client::operator=(const Client& rhs)
{
    if (this == &rhs) {
        return *this;
    }
    httpreq = rhs.getHttpReq();
    httpres = rhs.getHttpRes();
    vServer = rhs.getVserver();
    fd = rhs.getFd();
    client_ip = rhs.getClientIp();
    port = rhs.getPort();
	last_recv_time = rhs.getLastRecvTime();
	is_req_end = rhs.isEndOfReq();
    return *this;
}

Client::~Client()
{}


void Client::setFd(int fd) {
	this->fd = fd;
}

void Client::setHttpReq(HttpReq httpreq){
	this->httpreq = httpreq;
}

void Client::setHttpRes(HttpRes httpres){
	this->httpres = httpres;
}
void Client::setVserver(const VirtualServer& vServer){
	this->vServer = vServer;
}

void Client::setClientIp(std::string client_ip) {
    this->client_ip = client_ip;
}

void Client::setPort(int port) {
    this->port = port;
}

void Client::setLastRecvTime(time_t now) {
	this->last_recv_time = now;
}

void Client::setEndOfReq(bool flag) {
	this->is_req_end = flag;
}

bool Client::isEndOfReq() const {
	return is_req_end;
}

int Client::getFd() const{
	return fd;
}

HttpReq Client::getHttpReq() const
{
    return httpreq;
}

HttpRes Client::getHttpRes() const {
	return httpres;
}

VirtualServer Client::getVserver() const{
	return vServer;
}

std::string Client::getClientIp() const {
    return client_ip;
}

int Client::getPort() const {
    return port;
}
time_t Client::getLastRecvTime() const {
	return last_recv_time;
}
