#include "virtualServer.hpp"

virtualServer::virtualServer()
{}

virtualServer::virtualServer(const virtualServer& src)
{
	this->listen = src.listen;
	this->server_name = src.server_name;
	this->path = src.path;
	this->locations = src.locations;
	this->error_page = src.error_page;
	this->uri2location = src.uri2location;
}

virtualServer& virtualServer::operator=(const virtualServer& rhs)
{
	if (this == &rhs) {
		return *this;
	}
	this->listen = rhs.listen;
	this->server_name = rhs.server_name;
	this->path = rhs.path;
	this->locations = rhs.locations;
	this->error_page = rhs.error_page;
	this->uri2location = rhs.uri2location;
	return *this;
}

virtualServer::~virtualServer()
{}

//void virtualServer::set_listen(std::string listen){
void virtualServer::set_listen(int listen){
	this->listen = listen;
}

void virtualServer::set_uri2location(std::map<std::string, Location> uri2location){
	this->uri2location = uri2location;
}

void virtualServer::set_server_name(std::string server_name){
	this->server_name = server_name;
}
//void virtualServer::set_index(std::string index){
//	this->index = index;
//}

void virtualServer::set_location(Location location){
	locations.push_back(location);
}

void virtualServer::set_root(std::string root){
	this->root = root;
}

std::map<std::string, Location> virtualServer::get_uri2location() const {
	return uri2location;
}

int virtualServer::get_listen() const{
//std::string virtualServer::get_listen() const{
	return listen;
}

std::string virtualServer::get_server_name() const{
	return server_name;
}
//std::string virtualServer::get_index(){
//	return index;
//}
std::vector<Location> virtualServer::get_locations() const{
	return locations;
}

std::string virtualServer::get_root() const{
	return root;
}


std::ostream& operator <<(std::ostream& stream, const virtualServer& obj) {
		const std::vector<Location> tmp = obj.get_locations();
		stream << "listen: " << obj.get_listen() << std::endl
		<< "server_name: " << obj.get_server_name() << std::endl
		<< "server root: " << obj.get_root() << std::endl
//		<< "index: " << obj.get_index() << std::endl
		<< "locations:" << tmp.size() << std::endl << std::endl;
		for (std::vector<Location>::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
			stream << "location: " << *it << std::endl;
		}
//		stream << "===== uri2location phase =====" << std::endl;
//		const std::map<std::string, Location> tmp2 = obj.get_uri2location();
//		for (std::map<std::string, Location>::const_iterator it = tmp2.begin();
//			it != tmp2.end(); it++) {
//			stream << "uri: " << it->first << std::endl;
//			stream << "location: " << it->second << std::endl;
//		}
		stream << std::endl;
		return stream;
}
