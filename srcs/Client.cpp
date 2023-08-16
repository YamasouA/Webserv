#include "Client.hpp"
#include "http/HttpRes.hpp"

Client::Client()
: httpreq()
{}

Client::Client(const Client& source)
:httpreq(source.getHttpReq()),
    httpres(source.getHttpRes()),
    vServer(source.getVserver()),
    client_ip(source.getClientIp()),
    port(source.getPort())
{
	this->fd = source.getFd();
}

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
    return *this;
}

Client::~Client()
{}


void Client::setFd(int fd) {
	this->fd = fd;
}

void Client::setHttpReq(httpReq httpreq){
	this->httpreq = httpreq;
}

void Client::setHttpRes(HttpRes httpres){
	this->httpres = httpres;
}
void Client::setVserver(const virtualServer& vServer){
	this->vServer = vServer;
}

void Client::setClientIp(std::string client_ip) {
    this->client_ip = client_ip;
}

void Client::setPort(int port) {
    this->port = port;
}

int Client::getFd() const{
	return fd;
}

httpReq Client::getHttpReq() const
{
    return httpreq;
}

HttpRes Client::getHttpRes() const {
	return httpres;
}

virtualServer Client::getVserver() const{
	return vServer;
}

std::string Client::getClientIp() const {
    return client_ip;
}

int Client::getPort() const {
    return port;
}
