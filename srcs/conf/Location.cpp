#include "Location.hpp"

Location::Location(): depth(-1), max_body_size(-1) {
}
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
	return *this;
}

void Location::set_uri(std::string uri)
{
	this->uri = uri;
}

//void set_method(std::string method)
void Location::set_methods(std::vector<std::string> methods)
{
	this->methods = methods;
//	for (std::vector<std::string>::iterator it = this->methods.begin();
//		it != this->methods.end(); it++) {
//		std::cout << *it << std::endl;
//	}
}

void Location::set_root(std::string root)
{
	this->root = root;
}

void Location::set_is_autoindex(bool autoindex)
{
	this->autoindex = autoindex;
}

void Location::set_upload_path(std::string upload_path)
{
	this->upload_path = upload_path;
}

//void Location::set_index(std::string index)
void Location::set_index(std::vector<std::string> index)
{
	this->index = index;
}

//void Location::set_max_body_size(size_t max_body_size)
void Location::set_max_body_size(int max_body_size)
{
	this->max_body_size = max_body_size;
}

void Location::set_cgi_path(std::string cgi_path)
{
	this->cgi_path = cgi_path;
}

void Location::set_return(std::string ret)
{
	this->ret = ret;
}

void Location::set_location(Location location){
	locations.push_back(location);
}

void Location::set_depth(int depth){
	this->depth = (depth);
}

void Location::set_alias(std::string alias)
{
	this->alias = alias;
}

void Location::set_cgi_ext(std::vector<std::string> tokens) {
	this->cgi_ext = tokens;
}

void Location::set_error_pages(std::vector<std::string> tokens)
{
//	std::cout << "tokens: " << tokens[0] << " " << tokens[1] << std::endl;
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
//		std::cout << "status: " << status_code << std::endl;
		if (ss.fail() || status_code > 999) {
			continue;
		}
		error_pages[status_code] = path;
		std::cout << error_pages.size() << std::endl;
	}
}
std::string Location::get_uri() const{
	return uri;
}
std::vector<std::string> Location::get_methods() const{
	return methods;
}
std::string Location::get_root() const{
	return root;
}
bool Location::get_is_autoindex() const{
	return autoindex;
}
std::string Location::get_upload_path() const{
	return upload_path;
}
//std::string Location::get_index() const{
std::vector<std::string> Location::get_index() const{
	return index;
}
//size_t Location::get_max_body_size() const {
int Location::get_max_body_size() const {
	return max_body_size;
}

std::string Location::get_cgi_path() const {
	return cgi_path;
}

std::string Location::get_return() const {
	return ret;
}

std::vector<Location> Location::get_locations() const{
	return locations;
}

int Location::get_depth() const{
	return depth;
}

std::string Location::get_alias() const {
	return alias;
}

std::string Location::get_error_page(int status_code) const{
	std::map<int, std::string>::const_iterator it = error_pages.find(status_code);
	if (it != error_pages.end())
		return it->second;
	return "";
}

std::map<int, std::string> Location::get_error_pages() const{
	return error_pages;
}

std::vector<std::string> Location::get_cgi_ext() const {
	return cgi_ext;
}

std::ostream& operator <<(std::ostream& stream, const Location& obj) {
			const std::vector<std::string> tmp = obj.get_methods();
            const std::vector<std::string> tmp4 = obj.get_index();
			stream << "====== Location data =====" << std::endl
			<< "uri: " << obj.get_uri() << std::endl
			<< "methods: ";
			for (std::vector<std::string>::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
				stream << *it << " ";
			}
			stream << std::endl;
			stream << "location root: " << obj.get_root() << std::endl
			<< "is_autoindex: " << obj.get_is_autoindex() << std::endl
			<< "upload_path: " << obj.get_upload_path() << std::endl
			<< "index: ";// << obj.get_index() << std::endl
			for (std::vector<std::string>::const_iterator it2 = tmp4.begin(); it2 != tmp4.end(); ++it2) {
				stream << *it2 << " ";
			}
            stream << std::endl;
			stream << "max_body_size: " << obj.get_max_body_size() << std::endl
			<< "cgi_path: " << obj.get_cgi_path() << std::endl
			<< "return: " << obj.get_cgi_path() << std::endl;
			std::map<int, std::string>map = obj.get_error_pages();
			std::map<int, std::string>::iterator it= map.begin();
			for (; it != map.end(); it++) {
				stream << "status_code: " << it->first
				<< ", path: " << it->second << std::endl;
			}
			stream << "cgi_extension: ";
			const std::vector<std::string> tmp3 = obj.get_cgi_ext();
			for (std::vector<std::string>::const_iterator it = tmp3.begin(); it != tmp3.end(); ++it) {
				stream << *it << " ";
			}
			stream << "locations: ";
			const std::vector<Location> tmp2 = obj.get_locations();
			for (std::vector<Location>::const_iterator it = tmp2.begin(); it != tmp2.end(); ++it) {
				stream << *it << " ";
			}
            stream << std::endl;
			return stream;
}
