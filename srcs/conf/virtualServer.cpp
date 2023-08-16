#include "virtualServer.hpp"

virtualServer::virtualServer()
:whichOneExist(0)
{}

virtualServer::virtualServer(const virtualServer& src)
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
    this->depth = src.depth;
    this->alias = src.alias;
    this->autoindex = src.autoindex;
    this->max_body_size = src.max_body_size;
    this->cgi_ext = src.cgi_ext;
}

virtualServer& virtualServer::operator=(const virtualServer& rhs)
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
    this->depth = rhs.depth;
    this->alias = rhs.alias;
    this->autoindex = rhs.autoindex;
    this->max_body_size = rhs.max_body_size;
    this->cgi_ext = rhs.cgi_ext;
	return *this;
}

virtualServer::~virtualServer()
{}

void virtualServer::setListen(int listen){
    this->listen.push_back(listen);
}

void virtualServer::setUri2location(std::map<std::string, Location> uri2location){
	this->uri2location = uri2location;
}

void virtualServer::setServerName(std::vector<std::string> server_name){
	this->server_names.insert(this->server_names.end(), server_name.begin(), server_name.end());
}

void virtualServer::setLocation(Location location){
	locations.push_back(location);
}

void virtualServer::setRoot(std::string root){
	this->root = root;
}

std::map<std::string, Location> virtualServer::getUri2Location() const {
	return uri2location;
}

std::vector<int> virtualServer::getListen() const{
	return listen;
}

std::vector<std::string> virtualServer::getServerNames() const{
	return server_names;
}
std::vector<Location> virtualServer::getLocations() const{
	return locations;
}

std::string virtualServer::getRoot() const{
	return root;
}



void virtualServer::setMethods(std::vector<std::string> methods)
{
	this->methods = methods;
}

void virtualServer::setIsAutoindex(bool autoindex)
{
	this->autoindex = autoindex;
}

void virtualServer::setUploadPath(std::string upload_path)
{
	this->upload_path = upload_path;
}

void virtualServer::setIndex(std::vector<std::string> index)
{
	this->index = index;
}

void virtualServer::setMaxBodySize(size_t max_body_size)
{
	this->max_body_size = max_body_size;
}

void virtualServer::setCgiPath(std::string cgi_path)
{
	this->cgi_path = cgi_path;
}

void virtualServer::setReturn(std::string ret)
{
	this->ret = ret;
}

void virtualServer::setDepth(int depth){
	this->depth = (depth);
}

void virtualServer::setAlias(std::string alias)
{
	this->alias = alias;
}

void virtualServer::setCgiExt(std::vector<std::string> tokens) {
	this->cgi_ext = tokens;
}

void virtualServer::setErrorPages(std::vector<std::string> tokens)
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
		if (ss.fail() || status_code > 999) {
			continue;
		}
		error_pages[status_code] = path;
		std::cout << error_pages.size() << std::endl;
	}
}


std::vector<std::string> virtualServer::get_methods() const{
	return methods;
}
bool virtualServer::getIsAutoindex() const{
	return autoindex;
}
std::string virtualServer::getUploadPath() const{
	return upload_path;
}
std::vector<std::string> virtualServer::getIndex() const{
	return index;
}
size_t virtualServer::getMaxBodySize() const {
	return max_body_size;
}

std::string virtualServer::getCgiPath() const {
	return cgi_path;
}

std::string virtualServer::getReturn() const {
	return ret;
}

int virtualServer::getDepth() const{
	return depth;
}

std::string virtualServer::getAlias() const {
	return alias;
}

std::string virtualServer::getErrorPage(int status_code) const{
	std::map<int, std::string>::const_iterator it = error_pages.find(status_code);
	if (it != error_pages.end())
		return it->second;
	return "";
}

std::map<int, std::string> virtualServer::getErrorPages() const{
	return error_pages;
}

std::vector<std::string> virtualServer::getCgiExt() const {
	return cgi_ext;
}

void virtualServer::appendIndex(std::vector<std::string> elems) {
    index.insert(index.end(), elems.begin(), elems.end());
}

void virtualServer::appendCgiExt(std::vector<std::string> elems) {
    cgi_ext.insert(cgi_ext.end(), elems.begin(), elems.end());
}

std::ostream& operator <<(std::ostream& stream, const virtualServer& obj) {
		const std::vector<Location> tmp = obj.getLocations();
		stream << "server root: " << obj.getRoot() << std::endl
		<< "locations:" << tmp.size() << std::endl << std::endl;
		for (std::vector<Location>::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
			stream << "location: " << *it << std::endl;
		}
		stream << std::endl;
		return stream;
}
