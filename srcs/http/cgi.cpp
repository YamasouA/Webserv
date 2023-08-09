#include "cgi.hpp"

Cgi::Cgi() {}

Cgi::Cgi(const httpReq& request, Location location)
:httpreq(request),
    target(location),
    envs(request.get_meta_variables())
{}

Cgi::Cgi(const Cgi& src)
:header_fields(src.getHeaderFields())
{
    (void)src;
}

Cgi& Cgi::operator=(const Cgi& rhs)
{
    if (this == &rhs) {
        return *this;
    }
    this->header_fields = rhs.getHeaderFields();
    return *this;
}

Cgi::~Cgi()
{}


std::map<std::string, std::string> Cgi::getHeaderFields() const {
    return header_fields;
}

std::string Cgi::getCgiBody() const {
    return cgi_body;
}

int Cgi::getResType() const {
    return resType;
}

std::string Cgi::join_path() {
    std::cerr << "===== join_path(cgi) =====" << std::endl;
	std::string path_root = target.get_root();
	std::string config_path  = target.get_uri();
    std::string script_name = envs["SCRIPT_NAME"];
	std::string alias;
	if ((alias = target.get_alias()) != "") {
		config_path = alias;
	}
	if ((path_root.size() && path_root[path_root.length() - 1] == '/') || path_root.size() == 0) {
		if (config_path.size() >= 1)
			config_path = config_path.substr(1);
	}
	if (config_path == "" || config_path[config_path.length() - 1] == '/') {
		//file_path = file_path.substr(1);
		if (script_name.size() >= 1) { //
			script_name = script_name.substr(1);
        }
	}
	std::cerr << "join_path: " << path_root + config_path + script_name << std::endl;
    std::cerr << "===== End join_path =====" << std::endl;
	return path_root + config_path + script_name;
}

//std::string Cgi::encode_uri() {
//	std::ostringstream rets;
//	for(size_t n = 0; n < url.size(); n++) {
//	  unsigned char c = (unsigned char)url[n];
//	  if (isalnum(c) || c == '_' || c == '.' || c == '/' )
//	    rets << c;
//	  else {
//	    char buf[8];
//	    sprintf(buf, "%02x", (int)c);
//	    rets << '%' << buf[0] << buf[1];
//	  }
//	}
//	return rets.str();
//}


void Cgi::fix_up() {
    //if exist message-body, must set CONTENT_LENGTH value [MUST]
	if (envs.count("CONTENT_LENGTH") == 0 && httpreq.getContentBody().length() > 0) {
//		throw new Error();
	}
    if (envs.count("CONTENT_TYPE") == 0 && httpreq.getContentBody().length() > 0) {
//        throw new Error();
    }
    if (envs.count("GETAWAY_INTERFACE") == 0 && envs["gateway_interface"] == "CGI/1.1") {
//        throw new Error();
    }
    if (envs.count("PATH_INFO") == 0) {
//        throw new Error();
    }
    if (envs.count("PATH_TRASLATED") == 0) {
//        throw new Error();
    }
    if (envs.count("REMOTE_ADDR") == 0) {
//        throw new Error();
    }
    if (envs.count("REMOTE_HOST") == 0) {
//        throw new Error();
    }
    if (envs.count("REQUEST_METHOD") == 0) {
//        throw new Error();
    }
    if (envs.count("REQUEST_NAME") == 0) {
//        throw new Error();
    }
    if (envs.count("SCRIPT_NAME") == 0) {
//        throw new Error();
    }
    if (envs.count("SERVER_NAME") == 0) {
//        throw new Error();
    }
    if (envs.count("SERVER_PORT") == 0) {
//        throw new Error();
    }
    if (envs.count("SERVER_PROTOCOL") == 0) {
//        throw new Error();
    }
    if (envs.count("SERVER_SOFTWARE") == 0) {
//        throw new Error();
    }

    //if exist message-body, must set CONTENT_TYPE value　セットされていない場合はスクリプトが受信したデータのmime型を決定しようと試みる可能性がある
    //  未知のままであれば、スクリプトは型を application/octet-stream とみなすかもしれないし、誤りとして拒絶するかもしれない
    //  リクエストにCONTENT_TYPEが存在した場合はsetしなければならない [MUST]
    //GATEWAY_INTERFACE is must set value [MUST] CGI/1.1

	//envs["gateway_interface"] = "CGI/1.1"; // move before cgi

    // PATH_INFO　文字大小保存 制限を課しても課さなくても良い
    // PATH_TRANSLATED  QUERY_STRINGとかこの辺りはhttpreqで処理した方が良さそう？
    // REMOTE_ADDR [MUST] acceptの第２引数で取得できそう？
    // REMOTE_HOST(完全修飾ドメイン名) [SHOULD] 多分hostname
	envs.erase("HTTP_HOST");
    // REMOTE_IDENT [MAY]
    // REMOTE_USER http認証をclientが求めている場合は[MUST]
    // REQUEST_METHOD [MUST] 文字大小区別
	//envs["REQUEST_METHOD"] = getMethod();
    // SCRIPT_NAME [MUST] CGIスクリプトを識別することができる(URL符号化されていない)URL path
    //  pahtがNULLの場合は値は省略可能だが変数はセットしなければならない PATH_INFO部はまったく含まれない
    //
    // SERVER_NAME [MUST] 文字大小を区別しないhostnameまたはネットワークアドレス
    // SERVER_PORT [MUST] clientからリクエストを受信したTCP/IP port番号
    // SERVER_PROTOCOL [MUST] CGIリクエストに使用されるアプリケーションプロトコルの名前とバーション。clientとの通信でserverが使用するプロトコルの
    //  バージョンと違ってもよい[MAY]
    //
    // SERVER_SOFTWARE [MUST] CGIリクエストを行い、ゲートウェイを実行するサーバーソフトウェアの名前とバージョン clientに報告されたサーバーの説明があれば
    //  それと同じであるべき [SHOULD]
    //
    //
    //
    // set sever info meta vars
}

// "AUTH_TYPE" | "CONTENT_LENGTH" |
// "CONTENT_TYPE" | "GATEWAY_INTERFACE" |
// "PATH_INFO" | "PATH_TRANSLATED" |
// "QUERY_STRING" | "REMOTE_ADDR" |
// "REMOTE_HOST" | "REMOTE_IDENT" |
// "REMOTE_USER" | "REQUEST_METHOD" |
// "SCRIPT_NAME" | "SERVER_NAME" |
// "SERVER_PORT" | "SERVER_PROTOCOL" |
// "SERVER_SOFTWARE" | scheme |
//  protocol-var-name | extension-var-name"


void Cgi::set_env() {
    std::map<std::string, std::string> header_fields = httpreq.getHeaderFields();
	std::map<std::string, std::string>::iterator it = header_fields.begin();
	for (; it != header_fields.end(); it++) {
		std::string envs_var = "HTTP_";
		std::string http_req_field;
		std::cout << "it->first: " << it->first << " it->second: " << it->second << std::endl;
		std::transform(it->first.begin(), it->first.end(), std::back_inserter(http_req_field), ::toupper);
		std::cout << "fix: " << http_req_field<< std::endl;
		std::replace(http_req_field.begin(), http_req_field.end(), '-', '_');
		envs_var += http_req_field;
		envs[envs_var] = it->second;
	}
	fix_up();
}

void Cgi::send_body_to_child() {
    if (httpreq.getContentBody().length() > 0) {
	    write(1, httpreq.getContentBody().c_str(), httpreq.getContentBody().length());
    }
}

void Cgi::run_handler() {
	char **envs_ptr;
    std::map<std::string, std::string>::iterator its = envs.begin();
    for (; its != envs.end(); ++its) {
        std::cerr << its->first << "=" << its->second << std::endl;;
    }

	envs_ptr = new char *[envs.size() + 1];
	std::map<std::string, std::string>::iterator it = envs.begin();
    std::vector<std::string> tmp_vec;
	int i = 0;
	for (; it != envs.end(); ++it) {
		std::string env_exp = it->first + "=" + it->second;
        tmp_vec.push_back(env_exp);
    }
    for (std::vector<std::string>::iterator vec_it = tmp_vec.begin(); vec_it != tmp_vec.end(); ++vec_it) {
        envs_ptr[i] = (char*)vec_it->c_str();
		i++;
    }
	envs_ptr[envs.size()] = 0;
    std::string path = join_path();
	if (execve(path.c_str(), NULL, envs_ptr) < 0) {
        std::cerr << "failed exec errno: " << errno << std::endl;
    }
}

void Cgi::fork_process() {
	pid_t pid;
	int fd[2];
	int fd2[2];

	pipe(fd);
	pipe(fd2);
	set_env();

	pid = fork();
	// 子プロセス
	if (pid == 0) {
//		set_signal_handler(SIGINT, SIG_DFL);
//		set_signal_handler(SIGQUIT, SIG_DFL);
		close(fd[1]);
		close(fd2[0]);
		dup2(fd[0], 0);
		dup2(fd2[1], 1);

		run_handler();
		exit(1);
	}
	close(fd[0]);
	close(fd2[1]);
	dup2(fd[1], 1);
	dup2(fd2[0], 0);
	send_body_to_child();
	close(fd[1]);
    char tmp_buf;
    while (read(0, &tmp_buf, 1) > 0) {
        buf += tmp_buf;
    }
	close(fd2[0]);
}

bool Cgi::isLocalRedirect() {
	if (header_fields.size() != 1)
		return false;
	if (header_fields["location"] == "" || cgi_body != "")
		return false;
	if (header_fields["location"][0] != '/')
		return false;
	return true;
}

bool Cgi::isClientRedirect() {
	if (header_fields["location"] == "")
		return false;
	std::string abs_path = header_fields["location"];
	if(abs_path.compare(0, 5, "https") != 0 &&
	abs_path.compare(0, 4, "http") != 0) {
		return false;
	}
	return true;
}

void Cgi::detectResType() {
	if (header_fields.count("content-type") == 1) {
		resType = DOCUMENT;
	} else if (isLocalRedirect()) {
		resType = LOCAL_REDIRECT;
	} else if (isClientRedirect()) {
		if (cgi_body == "")
			resType = CLIENT_REDIRECT;
		else
			resType = CLIENT_REDIRECT_WITH_DOC;
	} else {
	    resType = NO_MATCH_TYPE;
    }
	std::cout << "resType: " << resType << std::endl;
}

bool Cgi::checkHeaderEnd(size_t& idx)
{
    if (buf[idx] == '\015') {
        ++idx;
        if (!expect('\012', idx)) {
			return false;
		}
        return true;
    } else if (buf[idx] == '\012') {
        ++idx;
        return true;
    } else {
        return false;
    }
}

void Cgi::skipSpace(size_t& idx)
{
	while (buf[idx] == ' ' || buf[idx] == '\t') {
		++idx;
	}
}

void Cgi::setHeaderField(const std::string& name, const std::string value)
{
    this->header_fields.insert(std::make_pair(name, value));
}

void Cgi::trim(std::string& str)
{
	std::string::size_type left = str.find_first_not_of("\t ");
	if (left != std::string::npos) {
		std::string::size_type right = str.find_last_not_of("\t ");
		str = str.substr(left, right - left + 1);
	}
}
std::string Cgi::getToken(char delimiter, size_t& idx)
{
	std::string token = "";
	while(idx < buf.length()) {
		if (buf[idx] == delimiter) {
			break;
		}
		token += buf[idx];
		idx++;
	}
	if (idx == buf.length()) {
		return "";
	}
	if (!expect(delimiter, idx)) {
		return "";
	}
    if (token.find(' ') != std::string::npos) {
        return "";
    }
	return token;
}

std::string Cgi::getToken_to_eol(size_t& idx) {
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

std::string Cgi::getToken_to_eof(size_t& idx) {
	std::string body = "";
	while (idx < buf.length()) {
		body += buf[idx];
		idx++;
	}
	return body;
}

void Cgi::fixUp(int& status) {
    if (header_fields.size() == 0) {
        status = 502;
        return;
    }
    if (header_fields.count("status") == 1) {
		// ここもutil関数使いたい
        std::stringstream ss;
        ss << header_fields["status"];
        ss >> status;
        if (status < 100 || 600 <= status) {
            status = 502;
        }
    }
    detectResType();
	if (resType == NO_MATCH_TYPE)
		status = 502;
}

// util関数
std::string Cgi::toLower(std::string str) {
	std::string s="";
	for (size_t i = 0; i < str.length(); i++) {
		s += std::tolower(str[i]);
	}
	return s;
}

bool Cgi::expect(char c, size_t& idx)
{
    if (buf[idx] != c) {
        std::cerr << "no expected " << c << std::endl;
		return false;
    }
    ++idx;
	return true;
}

int Cgi::parse_cgi_response() {
    int status = 200;
    size_t idx = 0;
	while (idx < buf.length()) {
        if (checkHeaderEnd(idx)) {
            break;
        }
        std::string field_name = getToken(':', idx);
		if (field_name == "") {
			status = 502;
			return status;
		}
		std::cout << "field_name: " << field_name << std::endl;
        skipSpace(idx);
        std::string field_value = getToken_to_eol(idx);
		std::cout << "field_value: " << field_value<< std::endl;
        trim(field_value);
        setHeaderField(toLower(field_name), field_value);
    }
    cgi_body = getToken_to_eof(idx);
    fixUp(status);
    return status;
}

void Cgi::run_cgi() {
	int backup_stdin = dup(STDIN_FILENO);
	int backup_stdout = dup(STDOUT_FILENO);

	fork_process();

	dup2(backup_stdin, STDIN_FILENO);
	dup2(backup_stdout, STDOUT_FILENO);
	close(backup_stdin);
	close(backup_stdout);
}
