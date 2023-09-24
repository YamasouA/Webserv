#include "VirtualServer.hpp"

VirtualServer::VirtualServer()
:whichOneExist(0),
	autoindex(false),
	max_body_size(0)
{}

VirtualServer::VirtualServer(const VirtualServer& src)
{
	this->listen = src.listen;
	this->server_names = src.server_names;
	this->path = src.path;
	this->locations = src.locations;
	this->error_page = src.error_page;
	this->uri2location = src.uri2location;
    this->index = src.index;
    this->ret = src.ret;
    this->methods = src.methods;
    this->upload_path = src.upload_path;
    this->cgi_path = src.cgi_path;
    this->error_pages = src.error_pages;
    this->alias = src.alias;
    this->autoindex = src.autoindex;
    this->max_body_size = src.max_body_size;
    this->cgi_ext = src.cgi_ext;
}

VirtualServer& VirtualServer::operator=(const VirtualServer& rhs)
{
	if (this == &rhs) {
		return *this;
	}
	this->listen = rhs.listen;
	this->server_names = rhs.server_names;
	this->path = rhs.path;
	this->locations = rhs.locations;
	this->error_page = rhs.error_page;
	this->uri2location = rhs.uri2location;
    this->index = rhs.index;
    this->ret = rhs.ret;
    this->methods = rhs.methods;
    this->upload_path = rhs.upload_path;
    this->cgi_path = rhs.cgi_path;
    this->error_pages = rhs.error_pages;
    this->alias = rhs.alias;
    this->autoindex = rhs.autoindex;
    this->max_body_size = rhs.max_body_size;
    this->cgi_ext = rhs.cgi_ext;
	return *this;
}

VirtualServer::~VirtualServer()
{}

void VirtualServer::setListen(int listen){
    this->listen.push_back(listen);
}

void VirtualServer::setUri2location(std::map<std::string, Location> uri2location){
	this->uri2location = uri2location;
}

void VirtualServer::setServerName(std::vector<std::string> server_name){
	this->server_names.insert(this->server_names.end(), server_name.begin(), server_name.end());
}

void VirtualServer::setLocation(Location location){
	locations.push_back(location);
}

void VirtualServer::setRoot(std::string root){
	this->root = root;
}

std::map<std::string, Location> VirtualServer::getUri2Location() const {
	return uri2location;
}

std::vector<int> VirtualServer::getListen() const{
	return listen;
}

std::vector<std::string> VirtualServer::getServerNames() const{
	return server_names;
}
std::vector<Location> VirtualServer::getLocations() const{
	return locations;
}

std::string VirtualServer::getRoot() const{
	return root;
}



void VirtualServer::setMethods(std::vector<std::string> methods)
{
	this->methods = methods;
}

void VirtualServer::setIsAutoindex(bool autoindex)
{
	this->autoindex = autoindex;
}

void VirtualServer::setUploadPath(std::string upload_path)
{
	this->upload_path = upload_path;
}

void VirtualServer::setIndex(std::vector<std::string> index)
{
	this->index = index;
}

void VirtualServer::setMaxBodySize(size_t max_body_size)
{
	this->max_body_size = max_body_size;
}

void VirtualServer::setCgiPath(std::string cgi_path)
{
	this->cgi_path = cgi_path;
}

void VirtualServer::setReturn(std::string ret)
{
	this->ret = ret;
}

void VirtualServer::setAlias(std::string alias)
{
	this->alias = alias;
}

void VirtualServer::setCgiExt(std::vector<std::string> tokens) {
	this->cgi_ext = tokens;
}

void VirtualServer::setErrorPages(std::vector<std::string> tokens)
{
	// status_codeとpathは必ず存在する
	if (tokens.size() < 2) {
		return;
	}
	std::string path = tokens[tokens.size() - 1];
	tokens.pop_back();
	// pathとして正しくない
	if (path[0] != '/') {
		return;
	}
	for (std::vector<std::string>::iterator it = tokens.begin();
		it != tokens.end(); it++) {
		std::stringstream ss(*it);
		int status_code;
		ss >> status_code;
		if (ss.bad()) {
			std::cerr << "stream is broken" << std::endl;
			std::exit(1);
		}
		if (ss.fail() || status_code > 999) {
			continue;
		}
		error_pages[status_code] = path;
	}
}


std::vector<std::string> VirtualServer::get_methods() const{
	return methods;
}
bool VirtualServer::getIsAutoindex() const{
	return autoindex;
}
std::string VirtualServer::getUploadPath() const{
	return upload_path;
}
std::vector<std::string> VirtualServer::getIndex() const{
	return index;
}
size_t VirtualServer::getMaxBodySize() const {
	return max_body_size;
}

std::string VirtualServer::getCgiPath() const {
	return cgi_path;
}

std::string VirtualServer::getReturn() const {
	return ret;
}

std::string VirtualServer::getAlias() const {
	return alias;
}

std::string VirtualServer::getErrorPage(int status_code) const{
	std::map<int, std::string>::const_iterator it = error_pages.find(status_code);
	if (it != error_pages.end())
		return it->second;
	return "";
}

std::map<int, std::string> VirtualServer::getErrorPages() const{
	return error_pages;
}

std::vector<std::string> VirtualServer::getCgiExt() const {
	return cgi_ext;
}

void VirtualServer::appendIndex(std::vector<std::string> elems) {
    index.insert(index.end(), elems.begin(), elems.end());
}

void VirtualServer::appendCgiExt(std::vector<std::string> elems) {
    cgi_ext.insert(cgi_ext.end(), elems.begin(), elems.end());
}

std::ostream& operator <<(std::ostream& stream, const VirtualServer& obj) {
		const std::vector<Location> tmp = obj.getLocations();
		stream << "server root: " << obj.getRoot() << std::endl
		<< "locations:" << tmp.size() << std::endl << std::endl;
		for (std::vector<Location>::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
			stream << "location: " << *it << std::endl;
		}
		stream << std::endl;
		return stream;
}
