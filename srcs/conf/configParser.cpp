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
	this->serve_confs = src.get_serve_confs();
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

std::vector<virtualServer> configParser::get_serve_confs() const
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

std::string configParser::get_token_to_eol() {
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
		|| buf[idx] == '\015' || buf[idx] == '\012') { //|| buf[idx] == ';') {
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


//How handle case no semicolon ?
std::string configParser::getToken(char delimiter)
{
	std::string token = "";

	if (idx < buf.length() && buf[idx] == '#') {
		get_token_to_eol();
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

//stringstreamを使わない実装の方が高速なので後でそちらに変更もあり
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
//	bool is_set_method = false;
//	bool is_set_max_size = false;
//	bool is_set_autoindex = false;
    int whichOneExistInLoc = 0;

	skip();
	std::string uri = getToken('{');
	trim(uri);
	if (uri == "") {
		throw SyntaxException("uri syntax Error in parseLocation");
	}
	location.set_uri(uri);
	skip();
	while (idx < buf.size()) {
		skip();
		if (buf[idx] == '}') {
			break;
		}
		std::string directive = getToken(' ');
		skip();
		if (directive == "root") {
            if (location.get_root() != "" || location.get_alias() != "") {
                throw SyntaxException("Location: duplicate directive: " + directive);
            }
            whichOneExistInLoc |= kRootExist;
            //must single and not coexist with alias
			location.set_root(getToken(';'));
		} else if (directive == "index") {
            //can multiple
			location.set_index(methodsSplit(getToken(';'), ' '));
		} else if (directive == "return") {
            //can multiple, but first one is used
			location.set_return(getToken(';'));
		} else if (directive == "method") {
            //must single
            if (whichOneExistInLoc & kMethodExist) {
                throw SyntaxException("Location: duplicate directive: " + directive);
            }
            whichOneExistInLoc |= kMethodExist;
//			is_set_method = true;
			const std::string methods = getToken(';');
			location.set_methods(methodsSplit(methods, ' '));
			// ' 'か', 'でsplitしてベクターに変換して返す
		} else if (directive == "autoindex") {
            //must single
            if (whichOneExistInLoc & kAutoIndexExist) { //bool -> int
                throw SyntaxException("Location: duplicate directive: " + directive);
            }
//	        is_set_autoindex = true;
            whichOneExistInLoc |= kAutoIndexExist;
			location.set_is_autoindex(getToken(';')=="on");
			// autoindexはbooleanで持つ？
		} else if (directive == "upload_path") {
            if (location.get_upload_path() != "") {
                throw SyntaxException("Location: duplicate directive: " + directive);
            }
            whichOneExistInLoc |= kUploadPathExist;
			location.set_upload_path(getToken(';'));
			// ワンチャンupload_pathは公式のものじゃないかも
		} else if (directive == "max_body_size") {
            //must single
            if (whichOneExistInLoc & kMaxSizeExist) {
                throw SyntaxException("Location: duplicate directive: " + directive);
            }
//	        is_set_max_size = true;
            whichOneExistInLoc |= kMaxSizeExist;
			std::stringstream sstream(getToken(';'));
			size_t result;
			sstream >> result;
			if (sstream.fail() && std::numeric_limits<size_t>::max() == result) {
				std::cerr << "overflow" << std::endl;
			}
			location.set_max_body_size(result);
		} else if (directive == "alias") {
            //must single and not coexist with root
            if (location.get_alias() != "" || location.get_root() != "") {
                throw SyntaxException("Location: duplicate directive: " + directive);
            }
            whichOneExistInLoc |= kAliasExist;
			location.set_alias(getToken(';'));
		} else if (directive == "location") {
			std::cout << "location-location" << std::endl;
			location.set_location(parseLocation());
		} else if (directive == "error_page") {
            //can multiple, but first one is used when same status code
			const std::string pages = getToken(';');
			location.set_error_pages(methodsSplit(pages, ' '));
		} else if (directive == "cgi_ext") {
			const std::string exts = getToken(';');
			location.set_cgi_ext(methodsSplit(exts, ' '));
		} else if (directive == "") {
            // comment out
            continue;
        } else {
			throw SyntaxException("Location: no such directive: " + directive);
			return location;
		}
	}

//	if (!is_set_method) {
	if (!(whichOneExistInLoc & kMethodExist)) {
		location.set_methods(methodsSplit("POST GET DELETE", ' '));
	}
    location.setWhichOneExist(whichOneExistInLoc);
	expect('}');
	skip();
	return location;
}

void configParser::setUriToMap(std::string prefix, std::string prefix_root, Location location) {
	std::string locationRoot = location.get_root();;
	std::string locationUri = location.get_uri();
	//std::string path = prefix + locationRoot + locationUri;
	std::string path = prefix + locationUri;
	std::vector<Location> locations = location.get_locations();
	// rootはこの時点でLocationに入れる
	std::string root = (locationRoot != "") ? locationRoot: prefix_root;
	for (std::vector<Location>::iterator it = locations.begin();
		it != locations.end(); it++) {
		setUriToMap(path, root, *it);
	}
	//path = root + path;
	location.set_root(root);
	uri2location[path] = location;
}

void configParser::uriToMap(virtualServer& vServer) {
	std::vector<Location> locations = vServer.get_locations();
	std::string serverRoot = vServer.get_root();

	for (std::vector<Location>::iterator it = locations.begin();
		it != locations.end(); it++) {
		setUriToMap("", "", *it);
	}
	vServer.set_uri2location(uri2location);
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
			v_serv.set_listen(result);
		} else if (directive == "server_name") {
            //can multiple
			v_serv.set_server_name(getToken(';'));
		} else if (directive == "root") {
            //must single
            if (whichOneExistInServ & kRootExist) {
                throw SyntaxException("Server: duplicate directive: " + directive);
            }
            whichOneExistInServ |= kRootExist;
			v_serv.set_root(getToken(';'));
		} else if (directive == "location") {
			v_serv.set_location(parseLocation());
		} else if (directive == "") {
			continue;
		} else {
			throw SyntaxException("Server: no such directive: " + directive);
		}
	}
    v_serv.setWhichOneExist(whichOneExistInServ);
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
		std::vector<Location> locations = v_it->get_locations();
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
		if (v_it->get_listen() == 0) {
			throw ConfigValueException("virtualServer derective should have 0 ~ 65535 port number");
		}
		if (v_it->get_server_name() == "") {
			throw ConfigValueException("virtualServer derective should have servername");
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
		skip(); // 空白などの読み飛ばし
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
