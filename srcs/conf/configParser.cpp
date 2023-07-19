#include "configParser.hpp"

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


Location configParser::get_uri2location(std::string uri) const
{
	std::map<std::string, Location>::const_iterator loc = uri2location.find(uri);
	if (loc != uri2location.end()) {
		return loc->second;
	}
	/*
	if (uri2location.count(uri) == 1)
		return uri2location[uri];
	*/
	std::string path = uri;
	while (1) {
		std::string::size_type i = path.find('/');
		if (i == std::string::npos)
			break;
		path = path.substr(0, i);

		std::map<std::string, Location>::const_iterator loc = uri2location.find(uri);
		if (loc != uri2location.end()) {
			return loc->second;
		}
		/*
		if (uri2location.count(path) == 1)
			return uri2location[path];
		*/
	}

	// 何もマッチしなかった場合の挙動イズなに？
	return loc->second;
}


const std::string readConfFile(const std::string& file_name)
{
	std::ifstream ifs(file_name.c_str());
	if (!ifs) {
		std::cout << "ko" << std::endl;
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

//void configParser::trim(std::string& str)
//{
//	size_t i = 0;
////	std::cout << "trim str: " << str << std::endl;
//	while (str[i] == ' ' || str[i] == '\t') {
//		++i;
//		std::cout << i << std::endl;
//	}
//	str.erase(0, i);
////	std::cout << "trimed str: " << str << std::endl;
//}

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
//	while (buf[idx] == delimiter) {
//		++idx;
//	}
//	skip();
	if (idx < buf.length() && buf[idx] == '#') {
		get_token_to_eol();
		return "";
	}
	while(idx < buf.length()) {
		if (buf[idx] == delimiter) {
//			idx++;
			break;
		}
		token += buf[idx];
		idx++;
	}
	if (idx == buf.length()) {
		std::cout << "delimiter: " << delimiter << std::endl;
		std::cout << "token: " << token<< std::endl;
		std::cout << "ko getToken" << std::endl;
		throw SyntaxException("syntax error in getToken");
	}
//	std::cout << "token end" << buf[idx] << std::endl;
//	idx++;
//	std::cout << "token end2" << buf[idx] << std::endl;
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
	skip();
	// その他prefixは考慮するとめんどくさそう
	std::string uri = getToken('{');
	// 末尾に空白が入るかも(入らない可能性もある)
	trim(uri);
	if (uri == "") {
		throw SyntaxException("uri syntax error in parseLocation");
	}
	location.set_uri(uri);
	skip();
	while (idx < buf.size()) {
	//std::cout << "=== Location ===" << std::endl;
		skip();
		if (buf[idx] == '}') {
//			idx++;
			break;
		}
//		std::cout << buf[idx] << std::endl;
		std::string directive = getToken(' ');
//		std::cout << "loc directive: " << directive << std::endl;
//		if (directive[0] == '#') {
//			get_token_to_eol();
//			skip();
//			continue;
//		} //????????????
		skip();
		if (directive == "root") {
			location.set_root(getToken(';'));
		} else if (directive == "index") {
			location.set_index(getToken(';'));
		} else if (directive == "return") {
			location.set_return(getToken(';'));
		} else if (directive == "method") {
			const std::string methods = getToken(';');
			location.set_methods(methodsSplit(methods, ' '));
			// ' 'か', 'でsplitしてベクターに変換して返す
		} else if (directive == "autoindex") {
			location.set_is_autoindex(getToken(';')=="on");
			// autoindexはbooleanで持つ？
		} else if (directive == "upload_path") {
			location.set_upload_path(getToken(';'));
			// ワンチャンupload_pathは公式のものじゃないかも
		} else if (directive == "max_body_size") {
			std::stringstream sstream(getToken(';'));
			size_t result;
			sstream >> result;
			if (sstream.fail() && std::numeric_limits<size_t>::max() == result) {
				std::cerr << "overflow" << std::endl;
			}
			location.set_max_body_size(result);
		} else if (directive == "alias") {
			location.set_alias(getToken(';'));
		} else if (directive == "location") {
			std::cout << "location-location" << std::endl;
			location.set_location(parseLocation());
		} else if (directive == "error_page") {
			const std::string pages = getToken(';');
			location.set_error_pages(methodsSplit(pages, ' '));
		} else if (directive == "cgi_ext") {
			const std::string exts = getToken(';');
			location.set_cgi_ext(methodsSplit(exts, ' '));
		} else if (directive == "") {
            // comment out
            continue;
        } else {
			throw SyntaxException("directive syntax error in parseLocation");
//			std::cerr << "\033[1;31msyntax error in location\033[0m: " << directive << std::endl;
			// 適切な例外を作成して投げる
			return location;
		}
//		idx++;
	}
	expect('}');
	skip();
//	std::cout << location << std::endl;
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

//void configParser::parseServe(size_t i) {
virtualServer configParser::parseServe() {
	std::string directive;
	virtualServer v_serv;
	//std::string value;
	//size_t i = 0;
	while (idx < buf.size()) {
	//std::cout << "=== server ===" << std::endl;
		skip();
		if (buf[idx] == '}') {
			break;
		}
		directive = getToken(' ');
//		std::cout << "directive: " << directive << std::endl;
//		if (directive[0] == '#') {
//			std::cout << directive << std::endl;
//			get_token_to_eol();
//			skip();
//			continue;
//		}
		skip();
		//value = getToken(';');
		if (directive == "listen") {
			//v_serv.set_listen(getToken(';'));
			std::stringstream sstream(getToken(';'));
			int result;
			sstream >> result;
			if (sstream.fail() && std::numeric_limits<int>::max() == result) {
				std::cerr << "overflow" << std::endl;
			}
			v_serv.set_listen(result);
		} else if (directive == "server_name") {
			v_serv.set_server_name(getToken(';'));
		} else if (directive == "root") {
			v_serv.set_root(getToken(';'));
		} else if (directive == "location") {
			//Location location = parseLocation();
			v_serv.set_location(parseLocation());
			//std::cout << "location" << std::endl<<v_serv.get_locations()[0] << std::endl;
		} else if (directive == "") {
			continue;
		} else {
//			continue;
			std::cerr << "\033[1;31mdirective Error\033[0m" << directive << std::endl;
		}
	}
//	std::cout << buf[idx] << std::endl;
	expect('}');
	skip();
	uriToMap(v_serv);
	return v_serv;
	//std::cout << "server: " << i <<  std::endl;
	//std::cout << serve_confs[i] << std::endl;
}

void configParser::expect(char c)
{
	if (buf[idx] != c) {
		std::cerr << "expected expression" << std::endl;
		std::exit(1);
	}
	++idx;
}

void configParser::parseConf()
{
	std::string directive;
	//std::string value;
	//size_t i = 0;

	directive = getToken('{');
	if (directive != "http") {
		std::cout << "Should http" << std::endl;
		std::exit(1);
	}
	while (idx < buf.size()) {
	//std::cout << "=== parseConf ===" << std::endl;
		if (buf[idx] == '}') {
			break;
		}
		directive = getToken('{');
		if (directive != "server") {
			std::cerr << "Error1" << std::endl;
			std::exit(1);
		}
		skip(); // 空白などの読み飛ばし
//		expect('{'); // 必須文字
		//virtualServer virtual_server = parseServe();
		//serve_confs.push_back(virtual_server);
		serve_confs.push_back(parseServe());
		//std::cout << "size: " << serve_confs.size() << std::endl;
		//std::cout << "server conf[" << i << "]" << std::endl;
		//std::cout << serve_confs[i] << std::endl;
		//std::cout << virtual_server << std::endl;
		//parseServe(i);
	//	i++;
	}
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

const char* configParser::SyntaxException::what(void) const throw() //noexcept c++11~
{
	return msg.c_str();
}

const char* configParser::DupulicateException::what(void) const throw() //noexcept c++11~
{
	return msg.c_str();
}
