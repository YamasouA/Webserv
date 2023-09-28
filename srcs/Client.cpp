#include "Client.hpp"

Client::Client()
: httpreq(),
	fd(0),
	last_recv_time(std::time(0)),
	last_connect_time(std::time(0)),
	is_req_end(false),
	is_send_res(false)
{}

Client::Client(const Client& source)
:httpreq(source.getHttpReq()),
    httpres(source.getHttpRes()),
    vServer(source.getVserver()),
	fd(source.getFd()),
    client_ip(source.getClientIp()),
    port(source.getPort()),
	last_recv_time(source.getLastRecvTime()),
	last_connect_time(source.getLastConnectTime()),
	is_req_end(source.isEndOfReq()),
	is_send_res(source.isSendRes())
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
	last_connect_time = rhs.getLastConnectTime();
	is_req_end = rhs.isEndOfReq();
	is_send_res = rhs.isSendRes();
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

void Client::setLastConnectTime(time_t now) {
	this->last_connect_time = now;
}

void Client::setEndOfReq(bool flag) {
	this->is_req_end = flag;
}

bool Client::isEndOfReq() const {
	return is_req_end;
}

void Client::setIsSendRes(bool flag) {
	this->is_send_res = flag;
}

bool Client::isSendRes() const {
	return is_send_res;
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
time_t Client::getLastConnectTime() const {
	return last_connect_time;
}
