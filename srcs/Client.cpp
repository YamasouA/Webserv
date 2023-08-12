#include "Client.hpp"
#include "http/HttpRes.hpp"

//void Client::set_httpReq(httpReq httpReq){
//	this->httpReq = httpReq;
//}

Client::Client()
: httpreq()
{}

Client::Client(const Client& source)
:httpreq(source.get_httpReq()),
    httpres(source.get_httpRes()),
    vServer(source.get_vServer()),
    client_ip(source.get_client_ip()),
    port(source.get_port())
    //fd(source.get_fd())
{
	this->fd = source.get_fd();
}

Client& Client::operator=(const Client& rhs)
{
    if (this == &rhs) {
        return *this;
    }
    httpreq = rhs.get_httpReq();
    httpres = rhs.get_httpRes();
    vServer = rhs.get_vServer();
    fd = rhs.get_fd();
    client_ip = rhs.get_client_ip();
    port = rhs.get_port();
    return *this;
}

Client::~Client()
{}


void Client::set_fd(int fd) {
	this->fd = fd;
}

void Client::set_httpReq(httpReq httpreq){
	this->httpreq = httpreq;
}

void Client::set_httpRes(HttpRes httpres){
	this->httpres = httpres;
}
void Client::set_vServer(const virtualServer& vServer){
	this->vServer = vServer;
}

void Client::set_client_ip(std::string client_ip) {
    this->client_ip = client_ip;
}

void Client::set_port(int port) {
    this->port = port;
}

int Client::get_fd() const{
	return fd;
}

//httpReq Client::get_httpReq(){
//	return httpReq;
//}

//httpParser Client::get_parsedReq() const
httpReq Client::get_httpReq() const
{
    return httpreq;
}

HttpRes Client::get_httpRes() const {
	return httpres;
}

//HttpRes* Client::get_httpResp() const {
//    return httpres;
//}

virtualServer Client::get_vServer() const{
	return vServer;
}

std::string Client::get_client_ip() const {
    return client_ip;
}

int Client::get_port() const {
    return port;
}
