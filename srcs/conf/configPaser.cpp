#include "configPaser.hpp"

configPaser::configPaser(const std::string& strs)
:buf(strs),
	idx(0)
{}

configPaser::configPaser(const configPaser& src)
{}

configPaser& configPaser::operator=(const configPaser rhs)
{}

configPaser::~configPaser()
{}

const std::string readConfFile(const std::string& file_name)
{
	std::ifstream ifs(file_name);
	if (!ifs) {
		std::cout << "ko" << std::endl;
	}
	std::ostringstream oss;
	oss << ifs.rdbuf();
	const std::string strs;
	strs = oss.str();
	return strs;
}

std::string configPaser::get_token_to_eol() {
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

void skip()
{
	while (buf[idx] == ' ' || buf[idx] == '\t'
		|| buf[idx] == '\015' || buf[idx] == '\012') {
		++idx;
	}
}

std::string configPaser::getToken(char delimiter)
{
	std::string token = "";
	while(idx < buf.length()) {
		if (buf[idx] == delimiter) {
			break;
		}
		token += buf[idx];
		idx++;
	}
	if (idx == buf.length())
		std::cout << "ko" << std::endl;
		//throw ;
	}
	idx++;
	return token;
}

//stringstreamを使わない実装の方が高速なので後でそちらに変更もあり
static std::vector<std::string> methodsSplit(std::string strs, char delimi)
{
	vector<std::string> methods;
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
	location.set_uri(uri);
	skip();
	while (idx != buf.size()) {
		if (buf[idx] == '}') {
			idx++;
			break;
		}
		directive = getToken(' ');
		if (directive[0] == '#') {
			get_token_to_eol();
			skip();
			continue;
		}
		if (directive == "root") {
			location.set_root(getToken(';'));
		} else if (directive == "method") {
			methods = getToken(';');
			location.set_methods(split(methods));
			// ' 'か', 'でsplitしてベクターに変換して返す
		} else if (directive == "autoindex") {
			location.set_autoindex(getToken(';')=="on");
			// autoindexはbooleanで持つ？
		} else if (directive == "upload_path") {
			location.set_upload_path(get);
			// ワンチャンupload_pathは公式のものじゃないかも
		} else if (directive == "max_body_size") {
			std::stringstream sstream(getToken(';'));
			size_t result;
			sstream >> result;
			if (sstream.fail() && std::numeric_limits<size_t>::max() == result) {
				std::cerr << "overflow" << std::endl;
			}
			location.set_max_body_size(result);
		}
		idx++;
	}
	return location;
}

void configPaser::parseServe(size_t i) {
	std::string directive;
	//std::string value;
	//size_t i = 0;
	while (idx != buf.size()) {
		directive = getToken(' ');
		if (directive[0] == '#') {
			get_token_to_eol();
			skip();
			continue;
		}
		skip();
		//value = getToken(';');
		if (directive == "listen") {
			serve_confs[i].set_listen(getToken(";"));
		} else if (directive == "server_name") {
			serve_confs[i].set_server_name(getToken(";"))
		} else if (directive == "root") {
			serve_confs[i].set_root(getToken(";"));
		} else if (directive == "location") {
			serve_confs[i].set_location(parseLocation());
		} else {
			std::cerr << "directive Error" << std::endl;
		}
		if (buf[idx] == '}') {
			idx++;
			break;
		}
	}
	std::cout << server_confs[i] << std::endl;
}

void configPaser::paseConf()
{
	std::string directive;
	//std::string value;
	size_t i = 0;

	while (idx != buf.size()) {
		std::string directive = getToken(' ');
		if (directive != "server") {
			std::cerr << "Error1" << std::endl;
			std::exit(1);
		}
		skip(); // 空白などの読み飛ばし
		expect('{'); // 必須文字
		serve_confs.push_back(new virtualServer());
		parseServe(i);
		i++;
	}
}


