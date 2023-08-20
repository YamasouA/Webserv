#include "Location.hpp"

Location::Location()
: depth(-1),
	autoindex(false),
	max_body_size(-1),
	which_one_exist(0)
{}

Location::Location(const Location& src) {
	this->uri = src.uri;
	this->root = src.root;
	this->index = src.index;
	this->ret = src.ret;
	this->methods = src.methods;
	this->upload_path = src.upload_path;
	this->autoindex = src.autoindex;
	this->max_body_size = src.max_body_size;
	this->locations = src.locations;
	this->depth = src.depth;
	this->alias = src.alias;
	this->error_pages = src.error_pages;
	this->cgi_ext = src.cgi_ext;
    this->which_one_exist = src.which_one_exist;
}

Location& Location::operator=(const Location& src)
{
	if (this == &src) {
		return *this;
	}

	this->uri = src.uri;
	this->root = src.root;
	this->index = src.index;
	this->ret = src.ret;
	this->methods = src.methods;
	this->upload_path = src.upload_path;
	this->autoindex = src.autoindex;
	this->max_body_size = src.max_body_size;
	this->locations = src.locations;
	this->depth = src.depth;
	this->alias = src.alias;
	this->error_pages = src.error_pages;
	this->cgi_ext = src.cgi_ext;
    this->which_one_exist = src.which_one_exist;
	return *this;
}

void Location::setUri(std::string uri)
{
	this->uri = uri;
}

void Location::setMethods(std::vector<std::string> methods)
{
	this->methods = methods;
}

void Location::setRoot(std::string root)
{
	this->root = root;
}

void Location::setIsAutoindex(bool autoindex)
{
	this->autoindex = autoindex;
}

void Location::setUploadPath(std::string upload_path)
{
	this->upload_path = upload_path;
}

void Location::setIndex(std::vector<std::string> index)
{
	this->index = index;
}

void Location::setMaxBodySize(int max_body_size)
{
	this->max_body_size = max_body_size;
}

void Location::setCgiPath(std::string cgi_path)
{
	this->cgi_path = cgi_path;
}

void Location::setReturn(std::string ret)
{
	this->ret = ret;
}

void Location::setLocation(Location location){
	locations.push_back(location);
}

void Location::setDepth(int depth){
	this->depth = (depth);
}

void Location::setAlias(std::string alias)
{
	this->alias = alias;
}

void Location::setCgiExt(std::vector<std::string> tokens) {
	this->cgi_ext = tokens;
}

void Location::setErrorPages(std::vector<std::string> tokens)
{
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

void Location::setErrorPages(std::map<int, std::string> error_pages) {
    this->error_pages = error_pages;
}

void Location::setWhichOneExist(int which_one_exist) {
    this->which_one_exist = which_one_exist;
}
std::string Location::getUri() const{
	return uri;
}
std::vector<std::string> Location::get_methods() const{
	return methods;
}
std::string Location::getRoot() const{
	return root;
}
bool Location::getIsAutoindex() const{
	return autoindex;
}
std::string Location::getUploadPath() const{
	return upload_path;
}
std::vector<std::string> Location::getIndex() const{
	return index;
}
int Location::getMaxBodySize() const {
	return max_body_size;
}

std::string Location::getCgiPath() const {
	return cgi_path;
}

std::string Location::getReturn() const {
	return ret;
}

std::vector<Location> Location::getLocations() const{
	return locations;
}

int Location::getDepth() const{
	return depth;
}

std::string Location::getAlias() const {
	return alias;
}

std::string Location::getErrorPage(int status_code) const{
	std::map<int, std::string>::const_iterator it = error_pages.find(status_code);
	if (it != error_pages.end())
		return it->second;
	return "";
}

std::map<int, std::string> Location::getErrorPages() const{
	return error_pages;
}

std::vector<std::string> Location::getCgiExt() const {
	return cgi_ext;
}

int Location::getWhichOneExist() const {
    return which_one_exist;
}

void Location::appendIndex(std::vector<std::string> elems) {
    index.insert(index.end(), elems.begin(), elems.end());
}

void Location::appendCgiExt(std::vector<std::string> elems) {
    cgi_ext.insert(cgi_ext.end(), elems.begin(), elems.end());
}

std::ostream& operator <<(std::ostream& stream, const Location& obj) {
			const std::vector<std::string> tmp = obj.get_methods();
            const std::vector<std::string> tmp4 = obj.getIndex();
			stream << "====== Location data =====" << std::endl
			<< "uri: " << obj.getUri() << std::endl
			<< "methods: ";
			for (std::vector<std::string>::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
				stream << *it << " ";
			}
			stream << std::endl;
			stream << "location root: " << obj.getRoot() << std::endl
			<< "is_autoindex: " << obj.getIsAutoindex() << std::endl
			<< "upload_path: " << obj.getUploadPath() << std::endl
			<< "index: ";// << obj.getIndex() << std::endl
			for (std::vector<std::string>::const_iterator it2 = tmp4.begin(); it2 != tmp4.end(); ++it2) {
				stream << *it2 << " ";
			}
            stream << std::endl;
			stream << "max_body_size: " << obj.getMaxBodySize() << std::endl
			<< "cgi_path: " << obj.getCgiPath() << std::endl
			<< "return: " << obj.getCgiPath() << std::endl;
			std::map<int, std::string>map = obj.getErrorPages();
			std::map<int, std::string>::iterator it= map.begin();
			for (; it != map.end(); it++) {
				stream << "status_code: " << it->first
				<< ", path: " << it->second << std::endl;
			}
			stream << "cgi_extension: ";
			const std::vector<std::string> tmp3 = obj.getCgiExt();
			for (std::vector<std::string>::const_iterator it = tmp3.begin(); it != tmp3.end(); ++it) {
				stream << *it << " ";
			}
			stream << "locations: ";
			const std::vector<Location> tmp2 = obj.getLocations();
			for (std::vector<Location>::const_iterator it = tmp2.begin(); it != tmp2.end(); ++it) {
				stream << *it << " ";
			}
            stream << std::endl;
			return stream;
}
