#include "httpReq.hpp"

httpReq::httpReq()
{}

httpReq::httpReq(const std::string& request_msg)
:buf(request_msg),
    idx(0),
    redirect_cnt(0),
    keep_alive(0)
{}

httpReq::httpReq(const httpReq& src)
:client_ip(src.getClientIP()),
    port(src.getPort()),
    redirect_cnt(src.getRedirectCnt()),
    method(src.getMethod()),
    uri(src.getUri()),
    version(src.getVersion()),
    header_fields(src.getHeaderFields()),
    cgi_envs(src.get_meta_variables()),
    content_body(src.getContentBody()),
	parse_error(false),
    keep_alive(src.getKeepAlive()),
	query_string(src.getQueryString())
{
    (void)src;
}

httpReq& httpReq::operator=(const httpReq& rhs)
{
    if (this == &rhs) {
        return *this;
    }
    this->client_ip = rhs.getClientIP();
    this->port = rhs.getPort();
    this->method = rhs.getMethod();
    this->uri = rhs.getUri();
    this->version = rhs.getVersion();
    this->header_fields = rhs.getHeaderFields();
    this->content_body = rhs.getContentBody();
    this->keep_alive = rhs.getKeepAlive();
    this->cgi_envs = rhs.get_meta_variables();
    this->redirect_cnt = rhs.getRedirectCnt();
    this->query_string = rhs.getQueryString();
    return *this;
}

httpReq::~httpReq()
{
}

void httpReq::setClientIP(std::string client_ip) {
    this->client_ip = client_ip;
}

void httpReq::setPort(int port) {
    this->port = port;
}

void httpReq::setMethod(const std::string& token)
{
    this->method = token;
}

void httpReq::setUri(const std::string& token)
{
    this->uri = token;
}

void httpReq::setVersion(const std::string& token)
{
    this->version = token;
}

void httpReq::setContentBody(const std::string& token)
{
    this->content_body = token;
}

void httpReq::setHeaderField(const std::string& name, const std::string value)
{
    this->header_fields.insert(std::make_pair(name, value));
}

std::string httpReq::getClientIP() const {
    return this->client_ip;
}

int httpReq::getPort() const {
    return this->port;
}

std::string httpReq::getMethod() const
{
    return this->method;
}

std::string httpReq::getUri() const
{
    return this->uri;
}

std::string httpReq::getVersion() const
{
    return this->version;
}

std::string httpReq::getContentBody() const
{
    return this->content_body;
}

std::map<std::string, std::string> httpReq::getHeaderFields() const
{
    return this->header_fields;
}

int httpReq::getContentLength() const
{
    return this->content_length;
}

int httpReq::getKeepAlive() const
{
    return this->keep_alive;
}

int httpReq::getRedirectCnt() const {
    return this->redirect_cnt;
}

std::string httpReq::getQueryString() const {
    return this->query_string;
}

bool httpReq::isRedirectLimit() {
	return redirect_cnt >= kRedirectLimit;
}

void httpReq::incrementRedirectCnt() {
	redirect_cnt++;
}

void httpReq::skipSpace()
{
	while (buf[idx] == ' ' || buf[idx] == '\t') {
		++idx;
	}
}

void httpReq::trim(std::string& str)
{
	std::string::size_type left = str.find_first_not_of("\t ");
	if (left != std::string::npos) {
		std::string::size_type right = str.find_last_not_of("\t ");
		str = str.substr(left, right - left + 1);
	}
}

bool httpReq::isSpace(char c) {
	if (c == '\f' || c == '\n' || c == ' '
		|| c == '\r' || c == '\t' || c == '\v') {
		return true;
	}
	return false;
}

void httpReq::expect(char c)
{
    if (buf[idx] != c) {
        std::cerr << "no expected Error" << c << std::endl;
    }
    ++idx;
}

std::string httpReq::getToken(char delimiter)
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
		throw SyntaxException("syntax Error in getToken");
	}
	expect(delimiter);
    if (token.find(' ') != std::string::npos) {
        std::cerr << "status 400" << std::endl;
    }
	return token;
}

std::string httpReq::getToken_to_eol() {
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

void httpReq::parseChunk() {
	int chunkSize;

    std::cout << "==================parse chunk==================" << buf << std::endl;
    std::string tmp = getToken_to_eol();
    if (tmp.find_first_not_of("0123456789abcdef") != std::string::npos) {
        std::cerr << "400 Bad request" << std::endl;
        return;
    }
	std::stringstream(tmp) >> std::hex >> chunkSize;
    std::cout << "size: " << chunkSize << std::endl;
	while (chunkSize > 0) {
		content_body += buf.substr(idx, chunkSize);
		idx += chunkSize;
        checkHeaderEnd();
		content_length += chunkSize;
        tmp = getToken_to_eol();
        if (tmp.find_first_not_of("0123456789abcdef") != std::string::npos) {
            std::cerr << "400 Bad request" << std::endl;
            return;
        }
	    std::stringstream(tmp) >> std::hex >> chunkSize;
	}
	// discard trailer fields
	getToken_to_eof();
	header_fields["Transfer-Encoding"].erase();
    return;
}

std::string httpReq::getToken_to_eof() {
	std::string body = "";
	while (idx < buf.length()) {
		body += buf[idx];
		idx++;
	}
	return body;
}

void httpReq::checkUri() {
	std::string::size_type query_pos = uri.find("?");
    std::string::size_type fragment_pos = uri.find("#");
	if (query_pos == std::string::npos) {
		return;
	}
    if (fragment_pos == std::string::npos) {
	    args = uri.substr(query_pos+1);
    } else {
        args = uri.substr(query_pos + 1, fragment_pos);
//        fragment = uri.substr(fragment_pos + 1);
    }
	if (query_pos != std::string::npos) {
		query_string = uri.substr(query_pos + 1);
	}
    uri = uri.substr(0, query_pos);
}

void httpReq::parse_scheme() {
	if (uri.compare(0, 5, "https") == 0) {
        uri = uri.substr(6);
	} else if (uri.compare(0, 4, "http") == 0) {
        uri = uri.substr(5);
	} else {
        std::cerr << "invalid scheme Error" << std::endl;
	}
}

void httpReq::parse_host_port() {
    std::string host;
	size_t i = 0;
	size_t path_pos;
	std::string port_str;

    for (; uri[i]; ++i) {
        host += uri[i];
        if (uri[i] == ':' || uri[i] == '/') {
            break;
        }
    }
    if (host.length() <= 0) {
        std::cerr << "invalid host Error" << std::endl;
    }
    if (uri[i] == ':') {
        path_pos = uri.find('/');
        i = path_pos;
        if (path_pos != std::string::npos) {
            port_str = uri.substr(i + 1, path_pos);
            std::stringstream ss(port_str);
            int port_num;
            ss >> port_num;
			if (port_num < 0 || 65535 < port_num) {
        	    std::cerr << "invalid port Error" << std::endl;
        	}
        }
    }
    if (uri[i] != '/') {
        std::cerr << "path not found" << std::endl;
    }
    //path handle ...
	uri = uri.substr(i);
	checkUri();


    // :/が見つかるまでをhostとして切り取る :が見つかった場合はportの処理も行う
    // hostの長さが0で無いかのチェックとport番号が有効かのチェックを行う
    // host以降の始めが/だった場合uri(request-target)として切り取る checkuri呼べば良さそう?
    // host以降の始めが/ではなかった場合invalid format
}

void httpReq::parse_authority_and_path() {
	parse_host_port(); //関数に分けなくても良い?
}

void httpReq::absurl_parse() {
	parse_scheme();
	expect('/');
	expect('/');
	parse_authority_and_path();
}

void httpReq::fix_up() {
	if (header_fields.count("host") != 1) {
		std::cerr << "no host Error" << std::endl;
	}

	if (header_fields.count("connection") != 1) {
		std::cerr << "no connection Error" << std::endl;
	}
    if (header_fields["connection"] == "keep-alive") {
        keep_alive = 1;
    } else {
        keep_alive = 0;
    }
	if (header_fields.count("content-length") != 1 && content_body != "") {
		std::cerr << "no content-length " << std::endl;
        std::cerr << "411(Length Required)" << std::endl;
	}
    if (header_fields.count("content-length") >= 1 && header_fields.count("transfer-encoding") >= 1) {
        std::cerr << "400 (Bad Request)" << std::endl;
    }
    if (header_fields.count("trailer-encoding") == 1 && header_fields["trailer-encoding"] != "chunked") {
            std::cerr << "501(Not Implement)" << std::endl;
    }
	std::string content_length_s = header_fields["content-length"];
    std::stringstream ss(content_length_s);
    ss >> content_length;

	if (!(method == "GET" || method == "DELETE" || method == "POST")) {
		std::cerr << "501(Not Implement) " << std::endl;
	}
	if (uri.length() != 0 && uri[0] != '/') {
		absurl_parse();
	}
}

void httpReq::parseReqLine()
{
    method = getToken(' ');
    if (isSpace(buf[idx])) {
        std::cerr << "status 400" << std::endl;
    }
//    skipSpace();
    uri = getToken(' ');
	checkUri();
	if (uri.length() != 0 && uri[0] != '/') {
		absurl_parse();
	}
    if (isSpace(buf[idx])) {
        std::cerr << "status 400" << std::endl;
    }
//    skipSpace();
    version = buf.substr(idx, 8);
    idx += 8;
    if (version != "HTTP/1.1") { //tmp fix version
        std::cerr << "version Error" << std::endl;
    }
    if (buf[idx] == '\015') {
        ++idx;
        expect('\012');
    } else if (buf[idx] == '\012') {
        ++idx;
    } else {
        std::cerr << "invalid format" << std::endl;
    }
}

bool httpReq::checkHeaderEnd()
{
    if (buf[idx] == '\015') {
        ++idx;
        expect('\012');
        return true;
    } else if (buf[idx] == '\012') {
        ++idx;
        return true;
    } else {
        return false;
    }
}

std::string httpReq::toLower(std::string str) {
	std::string s="";
	for (size_t i = 0; i < str.length(); i++) {
		s += std::tolower(str[i]);
	}
	return s;
}

bool httpReq::hasObsFold(std::string str) {
	for (size_t i = 0; i < str.length(); i++) {
		// OWS CRLF RWS
		if (str[i] == '\015' && str[i + 1] && str[i + 1] == '\012') {
			if (str[i + 2] && isSpace(str[i + 2])) {
				return true;
			}
		}
	}
	return false;
}

static bool isVCHAR(std::string str) {
    for (std::string::const_iterator it = str.cbegin(); it != str.cend(); ++it) {
        if (*it <= 32 && 127 <= *it) {
            return false;
        }
    }
    return true;
}

void httpReq::checkFieldsValue() {
	for (std::map<std::string, std::string>::iterator it = header_fields.begin();
		it != header_fields.end(); it++) {
		if (hasObsFold(it->second)) {
			parse_error = true;
			return;
		}
        if (!(isVCHAR(it->second))) {
            parse_error = true;
            return;
        }
	}
}

void httpReq::skipEmptyLines() {
    while (1) {
        if (buf[idx] != ' ' && buf[idx] != '\t') {
            break;
        }
        skipSpace();
		if (buf[idx] == '\015') {
			if (buf[idx+1] == '\012') {
				idx += 2;
                continue;
			}
		} else if (buf[idx] == '\012') {
			idx++;
			continue;
		} else {
            std::cerr << "400 (Bad Request)" << std::endl;
        }
    }
}

void httpReq::parseRequest()
{
    skipEmptyLines();
    parseReqLine();
    while (idx < buf.size()) {
        if (checkHeaderEnd()) {
            break;
        }
        std::string field_name = getToken(':');
        skipSpace();
		std::string field_value = getToken_to_eol();
		trim(field_value);
        setHeaderField(toLower(field_name), field_value);
    }
	if (header_fields["transfer-encoding"] == "chunked") {
		parseChunk();
    } else {
		content_body = getToken_to_eof();
    }
    fix_up();
}

std::map<std::string, std::string> httpReq::get_meta_variables() const {
    return cgi_envs;
}

std::string httpReq::percent_encode() {
	std::ostringstream rets;
	for(size_t n = 0; n < query_string.size(); n++) {
	  unsigned char c = (unsigned char)query_string[n];
	  // query_stringでの予約文字
	  if (isalnum(c) || c == '+' || c == '&' || c == '=' )
	    rets << c;
	  else {
		rets << '%' << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << int(c);
	  }
	}
	return rets.str();
}

void httpReq::set_meta_variables(Location loc) {
    std::map<std::string, std::string> header_fields = getHeaderFields();
    if (header_fields.count("content-length") != 0) {
        cgi_envs["CONTENT_LENGTH"] = header_fields["content-length"]; //　メタ変数名後で大文字にする
    }
    if (header_fields.count("content-type") != 0) {
        cgi_envs["CONTENT_TYPE"] = header_fields["content-type"];
    }
    cgi_envs["GATEWAY_INTERFACE"] = "CGI/1.1";
	// Locationで取得したcgi拡張子とマッチするものがあるときにPATH_INFOを区切る
	std::vector<std::string> ext = loc.get_cgi_ext();
	for (std::vector<std::string>::iterator it = ext.begin(); it != ext.end(); it++) {
		std::string::size_type idx = uri.find(*it);
		size_t len = (*it).size();
		if (idx == std::string::npos)
			continue;
		if ((uri[idx + len] != '\0' && uri[idx + len] == '/') || uri[idx + len] == '\0') {
			std::cout << "SET_SCRIPT_NAME" << std::endl;
			cgi_envs["SCRIPT_NAME"] = uri.substr(0, idx + len);
			cgi_envs["PATH_INFO"] = uri.substr(idx + len);
			cgi_envs["PATH_TRANSLATED"] = loc.get_root() + cgi_envs["PATH_INFO"];
		}
	}
	cgi_envs["QUERY_STRING"] = percent_encode();
	std::cout << "envs: " << cgi_envs["QUERY_STRING"] << std::endl;
//    struct sockaddr_in client_addr = get_client_addr();
//    std::string client_ip_str = my_inet_ntop(client_addr, NULL, 0); // httpReqにclientがもっているsockaddr_inをread_request内で渡す
    cgi_envs["REMOTE_ADDR"] = getClientIP();//恐らくacceptの第二引数でとれる値; inet系使えないと無理では？
    cgi_envs["REMOTE_HOST"] = cgi_envs["REMOTE_ADDR"]; //REMOTE_ADDRの値の方が良さそう(DNSに毎回問い合わせる重い処理をサーバー側でやらない方が良さげなので)
//    cgi_envs["REMOTE_HOST"] = header_fields["host"]; //REMOTE_ADDRの値の方が良さそう(DNSに毎回問い合わせる重い処理をサーバー側でやらない方が良さげなので)
	cgi_envs["REQUEST_METHOD"] = getMethod();
    cgi_envs["SERVER_NAME"] = header_fields["host"];
	// util関数
    std::stringstream ss;
    std::string port_str;
    ss << getPort();
    ss >> port_str;
    cgi_envs["SERVER_PORT"] = port_str;//port番号; urlからparse時にportを保存する<= ではなくhtonsなどでsocketから取得する？ or config fileから?(一つのserverに複数portあった時が厳しい)
    cgi_envs["SERVER_PROTOCOL"] = "HTTP/1.1";
    cgi_envs["SERVER_SOFTWARE"] = "WebServe";
}

std::ostream& operator<<(std::ostream& stream, const httpReq& obj) {
//    const std::vector<httpReq> tmp = obj.getHeaderInfo();
    const std::map<std::string, std::string> tmp = obj.getHeaderFields();
    stream << "method: " << obj.getMethod() << std::endl
    << "uri: " << obj.getUri() << std::endl
    << "version" << obj.getVersion() << std::endl << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
        stream << "header field: " << (*it).first << std::endl
        << "value: " << (*it).second << std::endl;
    }
    stream << std::endl
    << "body: " << obj.getContentBody() << std::endl;
    return stream;
}

httpReq::SyntaxException::SyntaxException(const std::string& what_arg)
:msg(what_arg)
{}

httpReq::SyntaxException::~SyntaxException() throw()
{}

const char* httpReq::SyntaxException::what(void) const throw() //noexcept c++11~
{
	return msg.c_str();
}
