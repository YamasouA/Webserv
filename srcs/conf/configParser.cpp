#include "configParser.hpp"

configParser::configParser()
:idx(0)
{}

configParser::configParser(const std::string& strs)
:buf(strs),
	idx(0)
{}

configParser::configParser(const configParser& src)
{
	this->serve_confs = src.getServerConfs();
}

configParser& configParser::operator=(const configParser& rhs)
{
	if (this == &rhs) {
		return *this;
	}
	return *this;
}

configParser::~configParser()
{}

std::vector<virtualServer> configParser::getServerConfs() const
{
	return serve_confs;
}

void configParser::set_buf(std::string strs) {
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

std::string configParser::getTokenToEOL() {
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

void configParser::skip()
{
	while (buf[idx] == ' ' || buf[idx] == '\t'
		|| buf[idx] == '\015' || buf[idx] == '\012') {
		++idx;
	}
}

void configParser::trim(std::string& str)
{
	std::string::size_type left = str.find_first_not_of("\t \n");
	if (left != std::string::npos) {
		std::string::size_type right = str.find_last_not_of("\t \n");
		str = str.substr(left, right - left + 1);
	}
}


std::string configParser::getToken(char delimiter)
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

Location configParser::parseLocation() {
	Location location;
    int whichOneExistInLoc = 0;

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
            if (location.getRoot() != "" || location.getAlias() != "") {
                throw SyntaxException("Location: duplicate directive: " + directive);
            }
            whichOneExistInLoc |= kRootExist;
            //must single and not coexist with alias
			location.setRoot(getToken(';'));
		} else if (directive == "index") {
            //can multiple
            if (whichOneExistInLoc & kIndexExist) {
                location.appendIndex(methodsSplit(getToken(';'), ' '));
            } else {
                location.setIndex(methodsSplit(getToken(';'), ' '));
            }
            whichOneExistInLoc |= kIndexExist;
		} else if (directive == "return") {
            //can multiple, but first one is used
            if (whichOneExistInLoc & kReturnExist) {
                getToken(';');
                continue;
            }
			location.setReturn(getToken(';'));
            whichOneExistInLoc |= kReturnExist;
		} else if (directive == "method") {
            //must single
            if (whichOneExistInLoc & kMethodExist) {
                throw SyntaxException("Location: duplicate directive: " + directive);
            }
            whichOneExistInLoc |= kMethodExist;
			const std::string methods = getToken(';');
			location.setMethods(methodsSplit(methods, ' '));
		} else if (directive == "autoindex") {
            //must single
            if (whichOneExistInLoc & kAutoIndexExist) {
                throw SyntaxException("Location: duplicate directive: " + directive);
            }
            whichOneExistInLoc |= kAutoIndexExist;
			location.setIsAutoindex(getToken(';')=="on");
		} else if (directive == "upload_path") {
            if (location.getUploadPath() != "") {
                throw SyntaxException("Location: duplicate directive: " + directive);
            }
            whichOneExistInLoc |= kUploadPathExist;
			location.setUploadPath(getToken(';'));
		} else if (directive == "max_body_size") {
            //must single
            if (whichOneExistInLoc & kMaxSizeExist) {
                throw SyntaxException("Location: duplicate directive: " + directive);
            }
            whichOneExistInLoc |= kMaxSizeExist;
			std::stringstream sstream(getToken(';'));
			int result;
			sstream >> result;
			if (sstream.fail() && std::numeric_limits<int>::max() == result) {
				std::cerr << "overflow" << std::endl;
			}
			location.setMaxBodySize(result);
		} else if (directive == "alias") {
            if (location.getAlias() != "" || location.getRoot() != "") {
                throw SyntaxException("Location: duplicate directive: " + directive);
            }
            whichOneExistInLoc |= kAliasExist;
			location.setAlias(getToken(';'));
		} else if (directive == "location") {
			std::cout << "location-location" << std::endl;
			location.setLocation(parseLocation());
		} else if (directive == "error_page") {
            whichOneExistInLoc |= kErrorPageExist;
			const std::string pages = getToken(';');
			location.setErrorPages(methodsSplit(pages, ' '));
		} else if (directive == "cgi_ext") {
			const std::string exts = getToken(';');
            if (whichOneExistInLoc & kCgiExtExist) {
                location.appendCgiExt(methodsSplit(exts, ' '));
            } else {
                location.setCgiExt(methodsSplit(exts, ' '));
            }
            whichOneExistInLoc |= kCgiExtExist;
		} else if (directive == "") {
            continue;
        } else {
			throw SyntaxException("Location: no such directive: " + directive);
			return location;
		}
	}

	if (!(whichOneExistInLoc & kMethodExist)) {
		location.setMethods(methodsSplit("POST GET DELETE", ' '));
	}
    location.setWhichOneExist(whichOneExistInLoc);
	expect('}');
	skip();
	return location;
}

void configParser::setUriToMap(std::string prefix, std::string prefix_root, Location location, const virtualServer& v_serv) {
	std::string locationRoot = location.getRoot();;
	std::string locationUri = location.get_uri();
	std::string path = prefix + locationUri;
	std::vector<Location> locations = location.getLocations();
	std::string root = (locationRoot != "") ? locationRoot: prefix_root;
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

void configParser::uriToMap(virtualServer& vServer) {
	std::vector<Location> locations = vServer.getLocations();
	std::string serverRoot = vServer.getRoot();

	for (std::vector<Location>::iterator it = locations.begin();
		it != locations.end(); it++) {
		setUriToMap("", "", *it, vServer);
	}
	vServer.setUri2location(uri2location);
	std::cout << "vServer: " << vServer << std::endl;
}


virtualServer configParser::parseServe() {
	std::string directive;
	virtualServer v_serv;
    int whichOneExistInServ = 0;

	while (idx < buf.size()) {
		skip();
		if (buf[idx] == '}') {
			break;
		}
		directive = getToken(' ');
		skip();
		if (directive == "listen") {
            //can multiple
			std::stringstream sstream(getToken(';'));
			int result;
			sstream >> result;
			if (sstream.fail() && std::numeric_limits<int>::max() == result) {
				std::cerr << "overflow" << std::endl;
            }
			v_serv.setListen(result);
		} else if (directive == "server_name") {
            //can multiple
			v_serv.setServerName(methodsSplit(getToken(';'), ' '));
		} else if (directive == "root") {
            //must single
            if (whichOneExistInServ & kRootExist) {
                throw SyntaxException("Server: duplicate directive: " + directive);
            }
            whichOneExistInServ |= kRootExist;
			v_serv.setRoot(getToken(';'));
		} else if (directive == "index") {
            //can multiple
            if (whichOneExistInServ & kIndexExist) {
                v_serv.appendIndex(methodsSplit(getToken(';'), ' '));
            } else {
                v_serv.setIndex(methodsSplit(getToken(';'), ' '));
            }
            whichOneExistInServ |= kIndexExist;
		} else if (directive == "return") {
            //can multiple, but first one is used
            if (whichOneExistInServ & kReturnExist) {
                continue;
            }
			v_serv.setReturn(getToken(';'));
            whichOneExistInServ |= kReturnExist;
		} else if (directive == "method") {
            //must single
            if (whichOneExistInServ & kMethodExist) {
                throw SyntaxException("v_serv: duplicate directive: " + directive);
            }
            whichOneExistInServ |= kMethodExist;
			const std::string methods = getToken(';');
			v_serv.setMethods(methodsSplit(methods, ' '));
		} else if (directive == "autoindex") {
            if (whichOneExistInServ & kAutoIndexExist) { //bool -> int
                throw SyntaxException("v_serv: duplicate directive: " + directive);
            }
            whichOneExistInServ |= kAutoIndexExist;
			v_serv.setIsAutoindex(getToken(';')=="on");
		} else if (directive == "upload_path") {
            if (v_serv.getUploadPath() != "") {
                throw SyntaxException("v_serv: duplicate directive: " + directive);
            }
            whichOneExistInServ |= kUploadPathExist;
			v_serv.setUploadPath(getToken(';'));
		} else if (directive == "max_body_size") {
            if (whichOneExistInServ & kMaxSizeExist) {
                throw SyntaxException("v_serv: duplicate directive: " + directive);
            }
            whichOneExistInServ |= kMaxSizeExist;
			std::stringstream sstream(getToken(';'));
			size_t result;
			sstream >> result;
			if (sstream.fail() && std::numeric_limits<size_t>::max() == result) {
				std::cerr << "overflow" << std::endl;
			}
			v_serv.setMaxBodySize(result);
		} else if (directive == "alias") {
            if (v_serv.getAlias() != "" || v_serv.getRoot() != "") {
                throw SyntaxException("v_serv: duplicate directive: " + directive);
            }
            whichOneExistInServ |= kAliasExist;
			v_serv.setAlias(getToken(';'));
		} else if (directive == "error_page") {
			const std::string pages = getToken(';');
			v_serv.setErrorPages(methodsSplit(pages, ' '));
		} else if (directive == "cgi_ext") {
			const std::string exts = getToken(';');
            if (whichOneExistInServ & kCgiExtExist) {
                v_serv.appendCgiExt(methodsSplit(exts, ' '));
            } else {
                v_serv.setCgiExt(methodsSplit(exts, ' '));
            }
            whichOneExistInServ |= kCgiExtExist;
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
	if (!(whichOneExistInServ & kMethodExist)) {
		v_serv.setMethods(methodsSplit("POST GET DELETE", ' '));
	}
	expect('}');
	skip();
	uriToMap(v_serv);
	return v_serv;
}

void configParser::expect(char c)
{
	if (buf[idx] != c) {
		throw SyntaxException(std::string("expected ") + c + std::string(" but ") + buf[idx]);
	}
	++idx;
}

void configParser::checkLocation() {
	std::vector<virtualServer>::iterator v_it = serve_confs.begin();
	for (; v_it != serve_confs.end(); v_it++) {
		std::vector<Location> locations = v_it->getLocations();
		if (locations.size() == 0) {
			throw ConfigValueException("virtualServer derective should have more than 1 Location derective");
		}
		std::vector<Location>::iterator l_it = locations.begin();
		for (; l_it != locations.end(); l_it++) {
			if (l_it->get_uri() == "") {
				throw ConfigValueException("Location derective should have path");
			}
		}
	}
}

void configParser::checkServer() {
	if (serve_confs.size() == 0) {
		throw ConfigValueException("http derective should have more than 1 virtualServer derective");
	}
	std::vector<virtualServer>::iterator v_it = serve_confs.begin();
	for (; v_it != serve_confs.end(); v_it++) {
        std::vector<int> listen = v_it->getListen();
        if (listen.size() == 0) {
            throw ConfigValueException("virtualServer derective should have 0 ~ 65535 port number");
        }
        for (std::vector<int>::iterator l_it = listen.begin(); l_it != listen.end(); ++l_it) {
            if (*l_it == 0) {
                throw ConfigValueException("virtualServer derective should have 0 ~ 65535 port number");
            }
        }
        std::vector<std::string> server_names = v_it->getServerNames();
        if (server_names.size() == 0) {
            throw ConfigValueException("virtualServer derective should have servername");
        }
        for (std::vector<std::string>::iterator n_it = server_names.begin(); n_it != server_names.end(); ++n_it) {
            if (*n_it == "") {
                throw ConfigValueException("virtualServer derective should have servername");
            }
        }
	}
}

void configParser::fixUp() {
	checkServer();
	checkLocation();
}

void configParser::parseConf()
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

configParser::SyntaxException::SyntaxException(const std::string& what_arg)
:msg(what_arg)
{}

configParser::SyntaxException::~SyntaxException() throw()
{}

configParser::DupulicateException::DupulicateException(const std::string& what_arg)
:msg(what_arg)
{}

configParser::DupulicateException::~DupulicateException() throw()
{}

configParser::ConfigValueException::ConfigValueException(const std::string& what_arg)
:msg(what_arg)
{}

configParser::ConfigValueException::~ConfigValueException() throw()
{}

const char* configParser::SyntaxException::what(void) const throw() //noexcept c++11~
{
	return msg.c_str();
}

const char* configParser::DupulicateException::what(void) const throw() //noexcept c++11~
{
	return msg.c_str();
}

const char* configParser::ConfigValueException::what(void) const throw() //noexcept c++11~
{
	return msg.c_str();
}
