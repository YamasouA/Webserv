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

void virtualServer::set_listen(int listen){
    this->listen.push_back(listen);
}

void virtualServer::set_uri2location(std::map<std::string, Location> uri2location){
	this->uri2location = uri2location;
}

void virtualServer::set_server_name(std::vector<std::string> server_name){
	this->server_names.insert(this->server_names.end(), server_name.begin(), server_name.end());
}

void virtualServer::set_location(Location location){
	locations.push_back(location);
}

void virtualServer::set_root(std::string root){
	this->root = root;
}

std::map<std::string, Location> virtualServer::get_uri2location() const {
	return uri2location;
}

std::vector<int> virtualServer::get_listen() const{
	return listen;
}

std::vector<std::string> virtualServer::get_server_names() const{
	return server_names;
}
std::vector<Location> virtualServer::get_locations() const{
	return locations;
}

std::string virtualServer::get_root() const{
	return root;
}



void virtualServer::set_methods(std::vector<std::string> methods)
{
	this->methods = methods;
}

void virtualServer::set_is_autoindex(bool autoindex)
{
	this->autoindex = autoindex;
}

void virtualServer::set_upload_path(std::string upload_path)
{
	this->upload_path = upload_path;
}

void virtualServer::set_index(std::vector<std::string> index)
{
	this->index = index;
}

void virtualServer::set_max_body_size(size_t max_body_size)
{
	this->max_body_size = max_body_size;
}

void virtualServer::set_cgi_path(std::string cgi_path)
{
	this->cgi_path = cgi_path;
}

void virtualServer::set_return(std::string ret)
{
	this->ret = ret;
}

void virtualServer::set_depth(int depth){
	this->depth = (depth);
}

void virtualServer::set_alias(std::string alias)
{
	this->alias = alias;
}

void virtualServer::set_cgi_ext(std::vector<std::string> tokens) {
	this->cgi_ext = tokens;
}

void virtualServer::set_error_pages(std::vector<std::string> tokens)
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
bool virtualServer::get_is_autoindex() const{
	return autoindex;
}
std::string virtualServer::get_upload_path() const{
	return upload_path;
}
std::vector<std::string> virtualServer::get_index() const{
	return index;
}
size_t virtualServer::get_max_body_size() const {
	return max_body_size;
}

std::string virtualServer::get_cgi_path() const {
	return cgi_path;
}

std::string virtualServer::get_return() const {
	return ret;
}

int virtualServer::get_depth() const{
	return depth;
}

std::string virtualServer::get_alias() const {
	return alias;
}

std::string virtualServer::get_error_page(int status_code) const{
	std::map<int, std::string>::const_iterator it = error_pages.find(status_code);
	if (it != error_pages.end())
		return it->second;
	return "";
}

std::map<int, std::string> virtualServer::get_error_pages() const{
	return error_pages;
}

std::vector<std::string> virtualServer::get_cgi_ext() const {
	return cgi_ext;
}

void virtualServer::append_index(std::vector<std::string> elems) {
    index.insert(index.end(), elems.begin(), elems.end());
}

void virtualServer::append_cgi_ext(std::vector<std::string> elems) {
    cgi_ext.insert(cgi_ext.end(), elems.begin(), elems.end());
}

std::ostream& operator <<(std::ostream& stream, const virtualServer& obj) {
		const std::vector<Location> tmp = obj.get_locations();
		stream << "server root: " << obj.get_root() << std::endl
		<< "locations:" << tmp.size() << std::endl << std::endl;
		for (std::vector<Location>::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
			stream << "location: " << *it << std::endl;
		}
		stream << std::endl;
		return stream;
}
