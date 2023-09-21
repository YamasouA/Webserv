#include "ConfigParser.hpp"

ConfigParser::ConfigParser()
:idx(0)
{}

ConfigParser::ConfigParser(const std::string& strs)
:buf(strs),
	idx(0)
{}

ConfigParser::ConfigParser(const ConfigParser& src)
{
	this->serve_confs = src.getServerConfs();
}

ConfigParser& ConfigParser::operator=(const ConfigParser& rhs)
{
	if (this == &rhs) {
		return *this;
	}
	return *this;
}

ConfigParser::~ConfigParser()
{}

std::vector<VirtualServer> ConfigParser::getServerConfs() const
{
	return serve_confs;
}

void ConfigParser::set_buf(std::string strs) {
	buf = strs;
}

const std::string readConfFile(const std::string& file_name)
{
	std::ifstream ifs(file_name.c_str());
	if (!ifs) {
		throw std::exception();
	}
	std::ostringstream oss;
	oss << ifs.rdbuf();
	const std::string strs = oss.str();
	return strs;
}

std::string ConfigParser::getTokenToEOL() {
	std::string line = "";
	while (idx < buf.length()) {
		if (buf[idx] == '\015') {
			if (buf[idx+1] == '\012') {
				idx += 2;
				return line;
			}
		} else if (buf[idx] == '\012') {
			idx++;
			return line;
		}
		line += buf[idx];
		idx++;
	}
	return line;
}

void ConfigParser::skip()
{
	while (buf[idx] == ' ' || buf[idx] == '\t'
		|| buf[idx] == '\015' || buf[idx] == '\012') {
		++idx;
	}
}

void ConfigParser::trim(std::string& str)
{
	std::string::size_type left = str.find_first_not_of("\t \n");
	if (left != std::string::npos) {
		std::string::size_type right = str.find_last_not_of("\t \n");
		str = str.substr(left, right - left + 1);
	}
}


std::string ConfigParser::getToken(char delimiter)
{
	std::string token = "";

	if (idx < buf.length() && buf[idx] == '#') {
		getTokenToEOL();
		return "";
	}
	while(idx < buf.length()) {
		if (buf[idx] == delimiter) {
			break;
		}
		token += buf[idx];
		idx++;
	}
	if (idx == buf.length()) {
		throw SyntaxException("config no delimiter");
	}
	expect(delimiter);
	skip();
	trim(token);
	return token;
}

static std::vector<std::string> methodsSplit(const std::string &strs, char delimi)
{
	std::vector<std::string> methods;
	std::stringstream ss(strs);
	std::string method;

	while (std::getline(ss, method, delimi)) {
		if (!method.empty()) {
			methods.push_back(method);
		}
	}
	return methods;
}

void ConfigParser::handleRootInLoc(Location& location, int *which_one_exist) {
	if (location.getRoot() != "" || location.getAlias() != "") {
		throw SyntaxException("Location: duplicate directive: root");
	}
	*which_one_exist |= kRootExist;
	//must single and not coexist with alias
	location.setRoot(getToken(';'));
}

void ConfigParser::handleIndexInLoc(Location& location, int *which_one_exist) {
	//can multiple
	if (*which_one_exist & kIndexExist) {
		location.appendIndex(methodsSplit(getToken(';'), ' '));
	} else {
		location.setIndex(methodsSplit(getToken(';'), ' '));
	}
	*which_one_exist |= kIndexExist;
}

void ConfigParser::handleReturnInLoc(Location& location, int *which_one_exist) {
	//can multiple, but first one is used
	if (*which_one_exist & kReturnExist) {
		getToken(';');
		//                continue;
		return;
	}
	location.setReturn(getToken(';'));
	*which_one_exist |= kReturnExist;
}

void ConfigParser::handleMethodInLoc(Location& location, int *which_one_exist) {
	//must single
	if (*which_one_exist & kMethodExist) {
		throw SyntaxException("Location: duplicate directive: method");
	}
	*which_one_exist |= kMethodExist;
	const std::string methods = getToken(';');
	location.setMethods(methodsSplit(methods, ' '));
}

void ConfigParser::handleAutoindexInLoc(Location& location, int *which_one_exist) {
	//must single
	if (*which_one_exist & kAutoIndexExist) {
		throw SyntaxException("Location: duplicate directive: autoindex");
	}
	*which_one_exist |= kAutoIndexExist;
	location.setIsAutoindex(getToken(';')=="on");
}

void ConfigParser::handleUploadPathInLoc(Location& location, int *which_one_exist) {
	if (location.getUploadPath() != "") {
		throw SyntaxException("Location: duplicate directive: upload_path");
	}
	*which_one_exist |= kUploadPathExist;
	location.setUploadPath(getToken(';'));
}

void ConfigParser::handleMaxBodySizeInLoc(Location& location, int *which_one_exist) {
	//must single
	if (*which_one_exist & kMaxSizeExist) {
		throw SyntaxException("Location: duplicate directive: max_body_size");
	}
	*which_one_exist |= kMaxSizeExist;
	const std::string tmp(getToken(';'));
	if (tmp.find_first_not_of("0123456789") != std::string::npos) {
		throw ConfigValueException("v_serv: invalid value: max_body_size");
	}
	std::stringstream sstream(tmp);
	size_t result;
	sstream >> result;
	if (sstream.fail() && std::numeric_limits<size_t>::max() == result) {
		std::cerr << "overflow" << std::endl;
		throw ConfigValueException("Location: invalid value: max_body_size");
	}
	location.setMaxBodySize(result);
}

void ConfigParser::handleAliasInLoc(Location& location, int *which_one_exist) {
	if (location.getAlias() != "" || location.getRoot() != "") {
		throw SyntaxException("Location: duplicate directive: alias");
	}
	*which_one_exist |= kAliasExist;
	location.setAlias(getToken(';'));
}

void ConfigParser::handleErrorPageInLoc(Location& location, int *which_one_exist) {
	*which_one_exist |= kErrorPageExist;
	const std::string pages = getToken(';');
	location.setErrorPages(methodsSplit(pages, ' '));
}

void ConfigParser::handleCgiExtInLoc(Location& location, int *which_one_exist) {
	const std::string exts = getToken(';');
	if (*which_one_exist & kCgiExtExist) {
		location.appendCgiExt(methodsSplit(exts, ' '));
	} else {
		location.setCgiExt(methodsSplit(exts, ' '));
	}
	*which_one_exist |= kCgiExtExist;
}

Location ConfigParser::parseLocation() {
	Location location;
    int which_one_exist = 0;

	skip();
	std::string uri = getToken('{');
	trim(uri);
	if (uri == "") {
		throw SyntaxException("uri syntax Error in parseLocation");
	}
	location.setUri(uri);
	skip();
	while (idx < buf.size()) {
		skip();
		if (buf[idx] == '}') {
			break;
		}
		std::string directive = getToken(' ');
		skip();
		if (directive == "root") {
			handleRootInLoc(location, &which_one_exist);
		} else if (directive == "index") {
			handleIndexInLoc(location, &which_one_exist);
		} else if (directive == "return") {
			handleReturnInLoc(location, &which_one_exist);
		} else if (directive == "method") {
			handleMethodInLoc(location, &which_one_exist);
		} else if (directive == "autoindex") {
			handleAutoindexInLoc(location, &which_one_exist);
		} else if (directive == "upload_path") {
			handleUploadPathInLoc(location, &which_one_exist);
		} else if (directive == "max_body_size") {
			handleMaxBodySizeInLoc(location, &which_one_exist);
		} else if (directive == "alias") {
			handleAliasInLoc(location, &which_one_exist);
		} else if (directive == "location") {
			std::cout << "location-location" << std::endl;
			location.setLocation(parseLocation());
		} else if (directive == "error_page") {
			handleErrorPageInLoc(location, &which_one_exist);
		} else if (directive == "cgi_ext") {
			handleCgiExtInLoc(location, &which_one_exist);
		} else if (directive == "") {
            continue;
        } else {
			throw SyntaxException("Location: no such directive: " + directive);
			return location;
		}
	}

	if (!(which_one_exist & kMethodExist)) {
		location.setMethods(methodsSplit("POST GET HEAD DELETE", ' '));
	}
    location.setWhichOneExist(which_one_exist);
	expect('}');
	skip();
	return location;
}

void ConfigParser::setUriToMap(std::string prefix, std::string prefix_root, Location location, const VirtualServer& v_serv) {
	std::string location_root = location.getRoot();;
	std::string location_uri = location.getUri();
	std::string path = prefix + location_uri;
	std::vector<Location> locations = location.getLocations();
	std::string root = (location_root != "") ? location_root: prefix_root;
	for (std::vector<Location>::iterator it = locations.begin();
		it != locations.end(); it++) {
		setUriToMap(path, root, *it, v_serv);
	}
	location.setRoot(root);
	int whichOneExist = location.getWhichOneExist();
    std::cout << "which: " << whichOneExist << std::endl;
    if (!(whichOneExist & kRootExist)) {
        std::cout << "set root " << std::endl;
        location.setRoot(v_serv.getRoot());
    }
    if (!(whichOneExist & kMethodExist)) {
        std::cout << "set method " << std::endl;
        location.setMethods(v_serv.get_methods());
    }
    if (!(whichOneExist & kAutoIndexExist)) {
        std::cout << "set autoindex " << std::endl;
        location.setIsAutoindex(v_serv.getIsAutoindex());
    }
    if (!(whichOneExist & kUploadPathExist)) {
        std::cout << "set upload path " << std::endl;
        location.setUploadPath(v_serv.getUploadPath());
    }
    if (!(whichOneExist & kMaxSizeExist)) {
        std::cout << "set max body " << std::endl;
        location.setMaxBodySize(v_serv.getMaxBodySize());
    }
    if (!(whichOneExist & kAliasExist)) {
        std::cout << "set alias " << std::endl;
        location.setAlias(v_serv.getAlias());
    }
    if (!(whichOneExist & kIndexExist)) {
        location.setIndex(v_serv.getIndex());
    }
    if (!(whichOneExist & kReturnExist)) {
        location.setReturn(v_serv.getReturn());
    }
    if (!(whichOneExist & kErrorPageExist)) {
        location.setErrorPages(v_serv.getErrorPages());
    }
    if (!(whichOneExist & kCgiExtExist)) {
        location.setCgiExt(v_serv.getCgiExt());
    }
    std::cout << "in setUriToMap: " << location << std::endl;
	uri2location[path] = location;
}

void ConfigParser::uriToMap(VirtualServer& vServer) {
	std::vector<Location> locations = vServer.getLocations();
//	std::string server_root = vServer.getRoot();

	for (std::vector<Location>::iterator it = locations.begin();
		it != locations.end(); it++) {
		setUriToMap("", "", *it, vServer);
	}
	vServer.setUri2location(uri2location);
	std::cout << "vServer: " << vServer << std::endl;
}

void ConfigParser::handleListenInServ(VirtualServer& v_serv) {
	//can multiple
	const std::string tmp(getToken(';'));
	if (tmp.find_first_not_of("0123456789") != std::string::npos) {
		throw ConfigValueException("v_serv: invalid value: listen");

	}
	std::stringstream sstream(tmp);
	int result;
	sstream >> result;
	if ((sstream.fail() && std::numeric_limits<int>::max() == result) || result < 0 || result > 65535) {
		throw ConfigValueException("VirtualServer derective should have 0 ~ 65535 port number");
	}
	v_serv.setListen(result);
}

void ConfigParser::handleRootInServ(VirtualServer& v_serv, int *which_one_exist) {
	//must single
	if (*which_one_exist & kRootExist) {
		throw SyntaxException("Server: duplicate directive: root");
	}
	*which_one_exist |= kRootExist;
	v_serv.setRoot(getToken(';'));
}

void ConfigParser::handleIndexInServ(VirtualServer& v_serv, int *which_one_exist) {
	//can multiple
	if (*which_one_exist & kIndexExist) {
		v_serv.appendIndex(methodsSplit(getToken(';'), ' '));
	} else {
		v_serv.setIndex(methodsSplit(getToken(';'), ' '));
	}
	*which_one_exist |= kIndexExist;
}

void ConfigParser::handleReturnInServ(VirtualServer& v_serv, int *which_one_exist) {
	//can multiple, but first one is used
	if (*which_one_exist & kReturnExist) {
		return;
		//                continue;
	}
	v_serv.setReturn(getToken(';'));
	*which_one_exist |= kReturnExist;
}

void ConfigParser::handleMethodInServ(VirtualServer& v_serv, int *which_one_exist) {
	//must single
	if (*which_one_exist & kMethodExist) {
		throw SyntaxException("v_serv: duplicate directive: method");
	}
	*which_one_exist |= kMethodExist;
	const std::string methods = getToken(';');
	v_serv.setMethods(methodsSplit(methods, ' '));
}

void ConfigParser::handleAutoindexInServ(VirtualServer& v_serv, int *which_one_exist) {
	if (*which_one_exist & kAutoIndexExist) { //bool -> int
		throw SyntaxException("v_serv: duplicate directive: autoindex");
	}
	*which_one_exist |= kAutoIndexExist;
	v_serv.setIsAutoindex(getToken(';')=="on");
}

void ConfigParser::handleUploadPathInServ(VirtualServer& v_serv) {
	if (v_serv.getUploadPath() != "") {
		throw SyntaxException("v_serv: duplicate directive: upload_path");
	}
	v_serv.setUploadPath(getToken(';'));
}

void ConfigParser::handleMaxBodySizeInServ(VirtualServer& v_serv, int *which_one_exist) {
	if (*which_one_exist & kMaxSizeExist) {
		throw SyntaxException("v_serv: duplicate directive: max_body_size");
	}
	*which_one_exist |= kMaxSizeExist;
	const std::string tmp(getToken(';'));
	if (tmp.find_first_not_of("0123456789") != std::string::npos) {
		throw ConfigValueException("v_serv: invalid value: max_body_size");
	}
	std::stringstream sstream(tmp);
	size_t result;
	sstream >> result;
	if (sstream.fail() && std::numeric_limits<size_t>::max() == result) {
		std::cerr << "overflow" << std::endl;
		throw ConfigValueException("v_serv: invalid value: max_body_size");
	}
	v_serv.setMaxBodySize(result);
}

void ConfigParser::handleErrorPageInServ(VirtualServer& v_serv) {
	const std::string pages = getToken(';');
	v_serv.setErrorPages(methodsSplit(pages, ' '));
}

void ConfigParser::handleCgiExtInServ(VirtualServer& v_serv, int *which_one_exist) {
	const std::string exts = getToken(';');
	if (*which_one_exist & kCgiExtExist) {
		v_serv.appendCgiExt(methodsSplit(exts, ' '));
	} else {
		v_serv.setCgiExt(methodsSplit(exts, ' '));
	}
	*which_one_exist |= kCgiExtExist;
}

VirtualServer ConfigParser::parseServe() {
	std::string directive;
	VirtualServer v_serv;
    int which_one_exist = 0;

	while (idx < buf.size()) {
		skip();
		if (buf[idx] == '}') {
			break;
		}
		directive = getToken(' ');
		skip();
		if (directive == "listen") {
			handleListenInServ(v_serv);
		} else if (directive == "server_name") {
            //can multiple
			v_serv.setServerName(methodsSplit(getToken(';'), ' '));
		} else if (directive == "root") {
			handleRootInServ(v_serv, &which_one_exist);
		} else if (directive == "index") {
			handleIndexInServ(v_serv, &which_one_exist);
		} else if (directive == "return") {
			handleReturnInServ(v_serv, &which_one_exist);
		} else if (directive == "method") {
			handleMethodInServ(v_serv, &which_one_exist);
		} else if (directive == "autoindex") {
			handleAutoindexInServ(v_serv, &which_one_exist);
		} else if (directive == "upload_path") {
			handleUploadPathInServ(v_serv);
		} else if (directive == "max_body_size") {
			handleMaxBodySizeInServ(v_serv, &which_one_exist);
		} else if (directive == "error_page") {
			handleErrorPageInServ(v_serv);
		} else if (directive == "cgi_ext") {
			handleCgiExtInServ(v_serv, &which_one_exist);
		} else if (directive == "") {
            continue;
		} else if (directive == "location") {
			v_serv.setLocation(parseLocation());
		} else if (directive == "") {
			continue;
		} else {
			throw SyntaxException("Server: no such directive: " + directive);
		}
	}

	if (!(which_one_exist & kMethodExist)) {
		v_serv.setMethods(methodsSplit("POST GET HEAD DELETE", ' '));
	}
	expect('}');
	skip();
	uriToMap(v_serv);
	return v_serv;
}

void ConfigParser::expect(char c)
{
	if (buf[idx] != c) {
		throw SyntaxException(std::string("expected ") + c + std::string(" but ") + buf[idx]);
	}
	++idx;
}

void ConfigParser::checkLocation() {
	std::vector<VirtualServer>::iterator v_it = serve_confs.begin();
	for (; v_it != serve_confs.end(); v_it++) {
		std::vector<Location> locations = v_it->getLocations();
		if (locations.size() == 0) {
			throw ConfigValueException("VirtualServer derective should have more than 1 Location derective");
		}
		std::vector<Location>::iterator l_it = locations.begin();
		for (; l_it != locations.end(); l_it++) {
			if (l_it->getUri() == "") {
				throw ConfigValueException("Location derective should have path");
			}
		}
	}
}

void ConfigParser::checkServer() {
	if (serve_confs.size() == 0) {
		throw ConfigValueException("http derective should have more than 1 VirtualServer derective");
	}
	std::vector<VirtualServer>::iterator v_it = serve_confs.begin();
	for (; v_it != serve_confs.end(); v_it++) {
        std::vector<int> listen = v_it->getListen();
        if (listen.size() == 0) {
            throw ConfigValueException("VirtualServer derective should have 0 ~ 65535 port number");
        }
        for (std::vector<int>::iterator l_it = listen.begin(); l_it != listen.end(); ++l_it) {
            if (*l_it == 0) {
                throw ConfigValueException("VirtualServer derective should have 0 ~ 65535 port number");
            }
        }
        std::vector<std::string> server_names = v_it->getServerNames();
        if (server_names.size() == 0) {
            throw ConfigValueException("VirtualServer derective should have servername");
        }
        for (std::vector<std::string>::iterator n_it = server_names.begin(); n_it != server_names.end(); ++n_it) {
            if (*n_it == "") {
                throw ConfigValueException("VirtualServer derective should have servername");
            }
        }
	}
}

void ConfigParser::fixUp() {
	checkServer();
	checkLocation();
}

void ConfigParser::parseConf()
{
	std::string directive;

	directive = getToken('{');
	if (directive != "http") {
		throw SyntaxException("config file must begin with http derective");
	}
	while (idx < buf.size()) {
		if (buf[idx] == '}') {
			break;
		}
		directive = getToken('{');
		if (directive != "server") {
			throw SyntaxException("need server derective");
		}
		skip();
		serve_confs.push_back(parseServe());
	}
	expect('}');
	fixUp();
}

ConfigParser::SyntaxException::SyntaxException(const std::string& what_arg)
:msg(what_arg)
{}

ConfigParser::SyntaxException::~SyntaxException() throw()
{}

ConfigParser::DupulicateException::DupulicateException(const std::string& what_arg)
:msg(what_arg)
{}

ConfigParser::DupulicateException::~DupulicateException() throw()
{}

ConfigParser::ConfigValueException::ConfigValueException(const std::string& what_arg)
:msg(what_arg)
{}

ConfigParser::ConfigValueException::~ConfigValueException() throw()
{}

const char* ConfigParser::SyntaxException::what(void) const throw() //noexcept c++11~
{
	return msg.c_str();
}

const char* ConfigParser::DupulicateException::what(void) const throw() //noexcept c++11~
{
	return msg.c_str();
}

const char* ConfigParser::ConfigValueException::what(void) const throw() //noexcept c++11~
{
	return msg.c_str();
}
