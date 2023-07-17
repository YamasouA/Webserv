#include "cgi.hpp"

Cgi::Cgi() {}

Cgi::Cgi(const httpReq& request, Location location)
:httpreq(request),
    target(location),
    envs(request.get_meta_variables())
{
    std::map<std::string, std::string>::iterator it = envs.begin();
    std::cout << "envs: " << std::endl;
    for (; it != envs.end(); ++it) {
        std::cout << it->first << "=" << it->second << std::endl;;
    }
}

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

//std::string Cgi::join_path(std::string& script_name) {
std::string Cgi::join_path() {
    std::cerr << "===== join_path =====" << std::endl;
	std::string path_root = target.get_root();
	std::string config_path  = target.get_uri();
    std::string script_name = envs["SCRIPT_NAME"];
    std::cerr << "script_name: " << script_name << std::endl;
//	std::string file_path = script_name;
	/*
						|            request uri       |
		/User/root/path/ /config/location/ /file_path.html
		path_root         config_path     file_path
	*/
	std::cerr << "root: " << path_root << std::endl;
	std::cerr << "config: " << config_path << std::endl;
    std::cerr << "ok" << std::endl;
	std::cerr << "cgi file: " << script_name << std::endl;
	std::string alias;
	if ((alias = target.get_alias()) != "") {
		config_path = alias;
	}
    //std::cout << "not auto index" << std::endl;
    //std::cout << "file_path(in join_path): " << file_path << std::endl;
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
	//std::cout << "path: " << path_root + config_path + file_path << std::endl;
	std::cerr << "join_path: " << path_root + config_path + script_name << std::endl;
    std::cerr << "===== End join_path =====" << std::endl;
	return path_root + config_path + script_name;
}


//void Cgi::reset_env() {
//}

//bool Cgi::is_meta_var() {
//	std::map<std::string, std::string> envs;
//
//	for (; it != httpreq.end(); it++) {
//		if
//	}
//}

bool Cgi::check_meta_var(std::string var1, std::string var2) {
	// 必須の箇所にデータが入ってるか
	// フォーマットがあるやつはそれを確認

	if (var1 == "auth_type" || var1 ==  "content_length" || var1 ==  "content_type" ||
		var1 == "gateway_interface" || var1 ==  "path_info" || var1 ==  "path_translated" ||
		var1 == "query_string" ||  var1 == "remote_addr" ||  var1 == "remote_host" ||
		var1 == "remote_ident" || var1 == "remote_user" || var1 ==  "request_method" ||
		var1 ==  "script_name" || var1 == "server_name" || var1 == "server_port" ||
		var1 == "server_protocol" || var1 == "server_software") {
			if (var2 != "") {
				return true;
			}
	}
	return false;
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
    // httpreq will have meta var info with map<std::string, std::string>
    std::map<std::string, std::string> header_fields = httpreq.getHeaderFields();
	std::map<std::string, std::string>::iterator it = header_fields.begin();
//	char **env_ptr;
//	std::map<std::string, std::string> envs = httpreq.get_meta_variables();;

    // Protocol-Specific Meta-Variables [MUST] 名前がHTTP_で始まるメタ変数は使用されるプロトコルがHTTPであればclient request header filedから読んだ値
    //  を含む
    //  HTTPヘッダフィールド名は全て大文字に変換し、'-'は'_'に変換し、頭に"HTTP"を付けてメタ変数とする。同じ名前のヘッダフィールドが複数受信された場合、
    //  サーバーは同じ意味の一つの値に書き換えなければならない [MUST] サーバーは必要であればデータ表現(文字集合など)をCGIメタ変数で適切なように変更
    //  しなければならない [MUST]
    //  サーバーは受信した全てのヘッダフィールドのメタ変数を作成する必要はない。特に認証情報を伝搬するもの(Authorizationなど)や他のメタ変数が
    //  スクリプトから利用可能なもの(Content-Type、Content-Length)は削除すべき [SHOULD]
    //  サーバーはConnectionヘッダフィールドなどのクライアントとの通信に関係するだけのヘッダフィールドを削除してもよい [MAY]
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
	std::map<std::string, std::string>::iterator it2 = envs.begin();
	for (; it2 != envs.end(); it2++) {
		std::cout << "envs: " << it2->first  << " = " << it2->second << std::endl;
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
    std::cerr << "run_handler envs: " << std::endl;
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
        //std::cerr << "envs_ptr[i]: " << envs_ptr[i] << std::endl;
        //std::cerr << "i: " << i << std::endl;
		i++;
    }
//        tmp_vec.push_back(it->first + "=" + it->second);
//        std::cerr << "envs_ptr in vec: " << (char *)tmp_vec.back().c_str() << std::endl;
//        std::cerr << "envs_prt in loop: " << envs_ptr[i] << std::endl;
//        std::cerr << "env_exp: " << env_exp << std::endl;
//        envs_ptr[i] = new char[env_exp.length() + 1];
//		envs_ptr[i] = (char *)env_exp.c_str();
//	}
	envs_ptr[envs.size()] = 0;
    for (size_t j = 0; envs_ptr[j]; ++j) {
        std::cerr << "j:" << j << std::endl;
        std::cerr << "envs_ptr: " << envs_ptr[j] << std::endl;
    }
    //std::cerr << "SCRIPT_NAME: " << envs["SCRIPT_NAME"] << std::endl;
//    std::string tmp_script_name = envs["SCRIPT_NAME"];
//    std::string path = join_path(tmp_script_name);
    std::string path = join_path();
    //std::cerr << "SCRIPT_NAME after join: " << path.c_str() << std::endl;
	std::cout << "path: " << path << std::endl;
	if (execve(path.c_str(), NULL, envs_ptr) < 0) {
        std::cerr << "failed exec errno: " << errno << std::endl;
    }
}

void Cgi::fork_process() {
	pid_t pid;
	int fd[2];

	pipe(fd);
//	reset_env();
	set_env();

	pid = fork();
	// 子プロセス
	if (pid == 0) {
//		set_signal_handler(SIGINT, SIG_DFL);
//		set_signal_handler(SIGQUIT, SIG_DFL);
		dup2(fd[1], 1);


//        return;

		close(fd[1]);
		close(fd[0]);
		run_handler();
		exit(1);
	}
	send_body_to_child();
	dup2(fd[0], 0);
	close(fd[1]);
	close(fd[0]);
//	return pid;
}

// ==== tmp func in cgi ====
bool Cgi::checkHeaderEnd(size_t& idx)
{
    if (buf[idx] == '\015') {
        ++idx;
        expect('\012', idx);
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
		std::cout << "delimiter: " << delimiter << std::endl;
		std::cout << "token: " << token<< std::endl;
		std::cout << "ko getToken" << std::endl;
		throw SyntaxException("syntax error in getToken");
	}
	expect(delimiter, idx);
    if (token.find(' ') != std::string::npos) {
        std::cerr << "status 400" << std::endl;
    }
//	trim(token);
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

// ==== tmp func in cgi end ====

void Cgi::fixUp(int& status) {
    if (header_fields.size() == 0) {
        status = 502;
        return;
    }
    if (header_fields.count("Status") == 1) {
        std::stringstream ss;
        ss << header_fields["Status"];
        ss >> status;
//        if (status < 100 || 600 <= status) {
//            status = 502; //?
//        }
    }
//    if (header_fields.count("Content-Type") != 1) {
//        //
//    }

}

void Cgi::expect(char c, size_t& idx)
{
    if (buf[idx] != c) {
        std::cerr << "no expected " << c << std::endl;
    }
    ++idx;
}

//have status in cgi class?
int Cgi::parse_cgi_response() {
    int status = 200;
    size_t idx = 0;
    //for (; idx < buf.length(); ++idx) {
	while (idx < buf.length()) {
        if (checkHeaderEnd(idx)) {
            break;
        }
        std::string field_name = getToken(':', idx);
		std::cout << "field_name: " << field_name << std::endl;
        skipSpace(idx);
        std::string field_value = getToken_to_eol(idx);
		std::cout << "field_value: " << field_value<< std::endl;
        trim(field_value);
        setHeaderField(field_name, field_value);
    }
    cgi_body = getToken_to_eof(idx);
    fixUp(status);
    return status;
}

void Cgi::run_cgi() {
	int backup_stdin = dup(STDIN_FILENO);
	int backup_stdout = dup(STDOUT_FILENO);

    std::map<std::string, std::string>::iterator it = envs.begin();
    std::cout << "run cgi envs: " << std::endl;
    for (; it != envs.end(); ++it) {
        std::cout << it->first << "=" << it->second << std::endl;;
    }

	fork_process();
    char tmp_buf;
//    waitpid(-1, NULL, 0);

    //for (size_t i = 0; read(0, &tmp_buf, 1) > 0; ++i) {
    while (read(0, &tmp_buf, 1) > 0) {
        buf += tmp_buf;
    }
    std::cout << "cgi output: " << std::endl << buf << std::endl << std::endl;

	dup2(backup_stdin, STDIN_FILENO);
	dup2(backup_stdout, STDOUT_FILENO);
	close(backup_stdin);
	close(backup_stdout);
}

Cgi::SyntaxException::SyntaxException(const std::string& what_arg)
:msg(what_arg)
{}

Cgi::SyntaxException::~SyntaxException() throw()
{}

const char* Cgi::SyntaxException::what(void) const throw() //noexcept c++11~
{
	return msg.c_str();
}
