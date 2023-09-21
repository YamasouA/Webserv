#include "HttpReq.hpp"

HttpReq::HttpReq()
:idx(0),
    redirect_cnt(0),
	is_header_end(false),
	is_req_end(false),
	parse_error(false),
    keep_alive(0),
    content_length(0),
    err_status(0),
	is_in_chunk_data(false)
{}

HttpReq::HttpReq(const std::string& request_msg)
:buf(request_msg),
    idx(0),
    redirect_cnt(0),
	is_header_end(false),
	is_req_end(false),
	parse_error(false),
    keep_alive(0),
    content_length(0),
    err_status(0),
	is_in_chunk_data(false)
{}

HttpReq::HttpReq(const HttpReq& src)
:body_buf(src.body_buf),
	buf(src.buf),
	idx(src.idx),
	client_ip(src.getClientIP()),
    port(src.getPort()),
    redirect_cnt(src.getRedirectCnt()),
	is_header_end(src.isEndOfHeader()),
	is_req_end(src.isEndOfReq()),
    method(src.getMethod()),
    uri(src.getUri()),
    version(src.getVersion()),
    header_fields(src.getHeaderFields()),
    cgi_envs(src.get_meta_variables()),
    content_body(src.getContentBody()),
	parse_error(false),
    keep_alive(src.getKeepAlive()),
	query_string(src.getQueryString()),
    content_length(src.getContentLength()),
    err_status(src.getErrStatus()),
	chunk_size(src.getChunkedSize()),
	is_in_chunk_data(src.isInChunkData())
{
    (void)src;
}

HttpReq& HttpReq::operator=(const HttpReq& rhs)
{
    if (this == &rhs) {
        return *this;
    }
    this->client_ip = rhs.getClientIP();
    this->buf = rhs.buf;
    this->body_buf = rhs.body_buf;
    this->idx = rhs.idx;
    this->port = rhs.getPort();
    this->method = rhs.getMethod();
    this->uri = rhs.getUri();
    this->version = rhs.getVersion();
    this->header_fields = rhs.getHeaderFields();
    this->content_body = rhs.getContentBody();
    this->keep_alive = rhs.getKeepAlive();
    this->content_length = rhs.getContentLength();
    this->cgi_envs = rhs.get_meta_variables();
    this->redirect_cnt = rhs.getRedirectCnt();
    this->query_string = rhs.getQueryString();
    this->content_length = rhs.getContentLength();
    this->err_status = rhs.getErrStatus();
	this->is_header_end = rhs.isEndOfHeader();
	this->is_req_end = rhs.isEndOfReq();
	this->chunk_size = rhs.getChunkedSize();
	this->is_in_chunk_data = rhs.isInChunkData();
    return *this;
}

HttpReq::~HttpReq()
{
}

std::string HttpReq::getBuf() const{
	return this->buf;
}

void HttpReq::appendReq(char *str) {
	if (isEndOfHeader()) {
		appendBody(str);
	} else {
		appendHeader(str);
	}
}

void HttpReq::appendHeader(std::string str) {
	this->buf += str;
	std::cout << buf << std::endl;
	size_t nl_idx = buf.find("\r\n\r\n");
	if (nl_idx != std::string::npos) {
		is_header_end = true;
		appendBody(buf.substr(nl_idx + 4));
		buf = buf.substr(0, nl_idx + 4);
	}
}

void HttpReq::appendBody(std::string str) {
	body_buf += str;
}

bool HttpReq::isEndOfHeader() const {
	return is_header_end;
}

bool HttpReq::isEndOfReq() const {
	return is_req_end;
}

void HttpReq::setClientIP(std::string client_ip) {
    this->client_ip = client_ip;
}

void HttpReq::setPort(int port) {
    this->port = port;
}

void HttpReq::setMethod(const std::string& token)
{
    this->method = token;
}

void HttpReq::setUri(const std::string& token)
{
    this->uri = token;
}

void HttpReq::setVersion(const std::string& token)
{
    this->version = token;
}

void HttpReq::setContentBody(const std::string& token)
{
    this->content_body = token;
}

void HttpReq::rejectReq(int err_status) {
	setErrStatus(err_status);
	is_req_end = true;
}

void HttpReq::setHeaderField(const std::string& name, const std::string value)
{
    if (name == "host" && header_fields.count("host") == 1) {
		return rejectReq(400);
    } else if (header_fields.count(name) == 1) {
        header_fields[name] += " ," + value;
    } else {
        this->header_fields.insert(std::make_pair(name, value));
    }
}

void HttpReq::setErrStatus(int err_status) {
    this->err_status = err_status;
}

std::string HttpReq::getClientIP() const {
    return this->client_ip;
}

int HttpReq::getPort() const {
    return this->port;
}

std::string HttpReq::getMethod() const
{
    return this->method;
}

std::string HttpReq::getUri() const
{
    return this->uri;
}

std::string HttpReq::getVersion() const
{
    return this->version;
}

std::string HttpReq::getContentBody() const
{
	return this->content_body;
}

std::map<std::string, std::string> HttpReq::getHeaderFields() const
{
    return this->header_fields;
}

size_t HttpReq::getContentLength() const
{
    return this->content_length;
}

int HttpReq::getKeepAlive() const
{
    return this->keep_alive;
}

int HttpReq::getRedirectCnt() const {
    return this->redirect_cnt;
}

std::string HttpReq::getQueryString() const {
    return this->query_string;
}

int HttpReq::getErrStatus() const {
    return this->err_status;
}

bool HttpReq::isRedirectLimit() {
	return redirect_cnt >= kRedirectLimit;
}

void HttpReq::incrementRedirectCnt() {
	redirect_cnt++;
}

size_t HttpReq::getChunkedSize() const {
	return chunk_size;
}

bool HttpReq::isInChunkData() const {
	return is_in_chunk_data;
}

void HttpReq::skipSpace()
{
	while (buf[idx] == ' ' || buf[idx] == '\t') {
		++idx;
	}
}

static void trim(std::string& str)
{
	std::string::size_type left = str.find_first_not_of("\t ");
	if (left != std::string::npos) {
		std::string::size_type right = str.find_last_not_of("\t ");
		str = str.substr(left, right - left + 1);
	}
}

bool HttpReq::isSpace(char c) {
	if (c == '\f' || c == '\n' || c == ' '
		|| c == '\r' || c == '\t' || c == '\v') {
		return true;
	}
	return false;
}

int HttpReq::expect(char c)
{
    if (buf[idx] != c) {
        std::cerr << "no expected Error" << c << std::endl;
		rejectReq(400);
        return 1;
    }
    ++idx;
    return 0;
}

std::string HttpReq::getToken(char delimiter)
{
	std::string token = "";
	while(idx < buf.length()) {
		if (buf[idx] == delimiter) {
			break;
		} else if (buf[idx] == ' ' || buf[idx] == '\t') {
			rejectReq(400);
			return "";
		}
		token += buf[idx];
		idx++;
	}
	if (idx == buf.length()) {
		std::cout << "delimiter: " << delimiter << std::endl;
		std::cout << "token: " << token<< std::endl;
		std::cout << "ko getToken" << std::endl;
		rejectReq(400);
        return "";
	}
	if (expect(delimiter)) {
		rejectReq(400);
        return "";
    }
	return token;
}

std::string HttpReq::getUriToken(char delimiter)
{
	size_t uri_length = 0;
	std::string token = "";
	while(idx < buf.length() || uri_length <= 8000) {
		if (buf[idx] == delimiter) {
			break;
		}
		token += buf[idx];
		++idx;
		++uri_length;
	}
	if (idx == buf.length()) {
		std::cout << "delimiter: " << delimiter << std::endl;
		std::cout << "token: " << token<< std::endl;
		std::cout << "ko getToken" << std::endl;
        setErrStatus(400);
        return "";
	}
	if (uri_length > 8000) {
		setErrStatus(414);
		is_req_end = true;
		return "";
	}
	if (expect(delimiter)) {
        setErrStatus(400);
        return "";
    }
    if (token.find(' ') != std::string::npos) {
        setErrStatus(400);
        return "";
    }
	return token;
}

std::string HttpReq::getTokenToEOL() {
	std::string line = "";
	while (idx < buf.length()) {
		if (buf[idx] == '\015') {
			if (buf[idx+1] == '\012') {
				idx += 2;
				return line;
			} else {
				rejectReq(400);
				return "";
			}
		} else if (buf[idx] == '\012') {
			idx++;
			return line;
		}
		line += buf[idx];
		idx++;
	}
	return "";
}

static const int RE_RECV = 1;
static const int OK = 0;
static const int ERROR = -1;


int HttpReq::getChunkSize() {
	std::cout << "===getChunkSize===" << std::endl;
	if (isInChunkData()) {
		return OK;
	}
	chunk_size = 0;
	size_t tmp_idx = idx;
	std::string tmp = "";
	while (idx < body_buf.length()) {
		if (body_buf[idx] == '\015') {
			if (body_buf[idx + 1] == '\012') {
				idx += 2;
				if (tmp == "" || tmp.find_first_not_of("0123456789abcdef") != std::string::npos) {
					std::cerr << "400 Bad request" << std::endl;
					rejectReq(400);
					return ERROR;
				}
				std::stringstream ss(tmp);
				ss >> std::hex >> chunk_size;
				if (ss.fail() && chunk_size == std::numeric_limits<size_t>::max()) {
					setErrStatus(400);
					is_req_end = true;
					return ERROR;
				}
				if (chunk_size + content_length > std::numeric_limits<size_t>::max()) {
					setErrStatus(413);
					is_req_end = true;
					return ERROR;
				}
				content_length += chunk_size;
				std::cout << "chunk_size: " << chunk_size << std::endl;
				return OK;
			} else if (body_buf[idx + 1] == '\0') {
				idx = tmp_idx;
				return RE_RECV;
			} else {
				std::cerr << "400 Bad request" << std::endl;
				rejectReq(400);
				return ERROR;
			}
		}
		tmp += body_buf[idx++];
	}
	idx = tmp_idx;
	return RE_RECV;
}

int HttpReq::getChunkData() {
	std::cout << "===getChunkData===" << std::endl;
	std::string tmp;
	tmp = body_buf.substr(idx);
	if (tmp.length() < chunk_size) {
		is_in_chunk_data = true;
		return RE_RECV;
	}
	if (tmp.substr(chunk_size).find("\r\n") == std::string::npos) {
		is_in_chunk_data = true;
		return RE_RECV;
	}
	content_body += tmp.substr(0, chunk_size);
	idx += chunk_size;
	while (1) {
		if (body_buf[idx] == '\015') {
			if (body_buf[idx + 1] == '\012') {
				idx += 2;
				is_in_chunk_data = false;
				return OK;
			}
		}
		++idx;
	}
}

void HttpReq::skipTokenToEOF() {
	while (idx < body_buf.length()) {
		++idx;
	}
	return;
}

void HttpReq::parseChunk() {
    std::cout << "==================parse chunk==================" << body_buf << std::endl;
	if (getChunkSize()) {
		return;
	}
	while (chunk_size > 0) {
		if (getChunkData()) {
			return;
		}
		if (getChunkSize()) {
			return;
		}
	}
    if (chunk_size == 0) {
		std::cout << "parsed body: " << content_body << std::endl;
		// discard trailer fields
		skipTokenToEOF();
		header_fields["Transfer-Encoding"].erase();
		is_req_end = true;
		return;
    }
}

std::string HttpReq::getTokenToEOF() {
	std::string body = "";
	while (idx < buf.length()) {
		body += buf[idx];
		idx++;
	}
	return body;
}

void HttpReq::checkUri() {
	std::string::size_type query_pos = uri.find("?");
    std::string::size_type fragment_pos = uri.find("#");
	if (query_pos == std::string::npos) {
		return;
	}
    if (fragment_pos == std::string::npos) {
	    args = uri.substr(query_pos+1);
    } else {
        args = uri.substr(query_pos + 1, fragment_pos);
    }
	if (query_pos != std::string::npos) {
		query_string = uri.substr(query_pos + 1);
	}
    uri = uri.substr(0, query_pos);
}

void HttpReq::parseScheme() {
	if (toLower(uri.substr(0, 5)).compare(0, 5, "https") == 0) {
        uri = uri.substr(6);
	} else if (toLower(uri.substr(0, 6)).compare(0, 4, "http") == 0) {
        uri = uri.substr(5);
	} else {
        std::cerr << "invalid scheme Error" << std::endl;
		rejectReq(400);
	}
}

void HttpReq::parseHostPort() {
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
		return rejectReq(400);
    }
    if (uri[i] == ':') {
        path_pos = uri.find('/');
        i = path_pos;
        if (path_pos != std::string::npos) {
            port_str = uri.substr(i + 1, path_pos);
            std::stringstream ss(port_str);
            int port_num;
            ss >> port_num;
			if (ss.bad()) {
				setErrStatus(500);
				return;
			}
			if (port_num < 0 || 65535 < port_num) {
        	    std::cerr << "invalid port Error" << std::endl;
				return rejectReq(400);
        	}
        }
    }
    if (uri[i] != '/') {
        std::cerr << "path not found" << std::endl;
		return rejectReq(400);
    }
    header_fields["host"] = host;
	uri = uri.substr(i);
	checkUri();
}

void HttpReq::parseAuthorityAndPath() {
	parseHostPort(); //関数に分けなくても良い?
}

void HttpReq::absUrlParse() {
	parseScheme();
    if (getErrStatus() > 0) {
        return;
    }
    if (uri[0] && uri[0] == '/' && uri[1] == '/') {
        uri.substr(2);
	    parseAuthorityAndPath();
    } else {
		rejectReq(400);
    }
}

static std::vector<std::string> fieldValueSplit(const std::string &strs, char delimi)
{
	std::vector<std::string> values;
	std::stringstream ss(strs);
	std::string value;

	while (std::getline(ss, value, delimi)) {
		if (!value.empty()) {
            trim(value);
			values.push_back(value);
		}
	}
	return values;
}


void HttpReq::fixUp() {
	std::cout << "=======req fixup ========" << std::endl;
	if (header_fields.count("host") != 1) {
		std::cerr << "no host Error" << std::endl;
		return rejectReq(400);
	}

	if (header_fields.count("connection") == 1) {
		if (header_fields["connection"] == "") {
			std::cerr << "no connection Error" << std::endl;
			return rejectReq(400);
		}
		std::vector<std::string> connections = fieldValueSplit(toLower(header_fields["connection"]), ',');
		std::vector<std::string>::iterator c_it = connections.begin();
		for (; c_it != connections.end(); c_it++) {
			if (*c_it == "close") {
				break;
			}
		}
		if (c_it == connections.end() || connections.size() == 0) {
			keep_alive = 1;
		} else
			keep_alive = 0;
	} else {
		keep_alive = 1;
	}

	std::cout << "keep_alive: " << keep_alive << std::endl;
	if (header_fields.count("content-length") != 1 && header_fields.count("transfer-encoding") != 1 && content_body != "") {
		std::cerr << "no content-length " << std::endl;
        std::cerr << "411(Length Required)" << std::endl;
		return rejectReq(411);
	}
    if (header_fields.count("content-length") == 1 && header_fields.count("transfer-encoding") == 1) {
        std::cerr << "400 (Bad Request)" << std::endl;
		return rejectReq(400);
    }
    if (header_fields.count("content-length") == 1) {
        if (header_fields["content-length"].find_first_not_of("0123456789") != std::string::npos) {
			return rejectReq(400);
        }
	    std::string content_length_s = header_fields["content-length"];
        std::stringstream ss(content_length_s);
        ss >> content_length;
		if (ss.bad()) {
			setErrStatus(500);
			return;
		}
		if (ss.fail() && (content_length == std::numeric_limits<size_t>::max())) {
            setErrStatus(413); //or 400
			return;
		}
    }

    if (content_body != "" && header_fields.count("content-type") != 1) {
        header_fields["content-type"] = "application/octet-stream";
    }
    if (header_fields.count("transfer-encoding") == 1) {
		std::vector<std::string> transfer_encodings = fieldValueSplit(toLower(header_fields["transfer-encoding"]), ',');
		std::vector<std::string>::iterator t_it = transfer_encodings.begin();
		for (; t_it != transfer_encodings.end(); t_it++) {
			if (*t_it != "chunked") {
			    std::cerr << "501(Not Implement) transfer-encoding" << std::endl;
				return rejectReq(501);
            }
		}
		header_fields["transfer-encoding"] = "chunked";
    }

	if (!(method == "GET" || method == "HEAD" || method == "DELETE" || method == "POST" || method == "PUT")) {
		std::cerr << "501(Not Implement) method" << std::endl;
		return rejectReq(501);
	}
	if (uri.length() != 0 && uri[0] != '/') {
		absUrlParse();
	}
}

void HttpReq::parseReqLine()
{
    method = getToken(' ');
    if (isSpace(buf[idx])) {
        std::cerr << "status 400" << std::endl;
		return rejectReq(400);
    }
    uri = getUriToken(' ');
	if (uri.length() == 0) {
		return;
	}
	checkUri();
	if (uri.length() != 0 && uri[0] != '/') {
		absUrlParse();
	}
    if (isSpace(buf[idx])) {
        std::cerr << "status 400" << std::endl;
		return rejectReq(400);
    }
    version = getTokenToEOL();
    if (version != "HTTP/1.1") { //tmp fix version
        std::cerr << "version Error" << std::endl;
		return rejectReq(505);
    }
}

bool HttpReq::checkHeaderEnd()
{
    if (buf[idx] == '\015') {
        ++idx;
        if (expect('\012')) {
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

std::string HttpReq::toLower(std::string str) {
	std::string s="";
	for (size_t i = 0; i < str.length(); i++) {
		s += std::tolower(str[i]);
	}
	return s;
}


static bool checkVCHAR(std::string str) {
	for (size_t i = 0; i < str.length(); i++) {
		if (str[i] <= 32 || 127 <= str[i]) {
    	    return false;
    	}
	}
    return true;
}

void HttpReq::skipEmptyLines() {
    while (1) {
        if (buf[idx] != ' ' && buf[idx] != '\t') {
            break;
        }
        skipSpace();
		if (buf[idx] == '\015') {
			if (buf[idx+1] == '\012') { // expect is better?
				idx += 2;
                continue;
			}
		} else if (buf[idx] == '\012') {
			idx++;
			continue;
		} else {
            std::cerr << "400 (Bad Request)" << std::endl;
			return rejectReq(400);
        }
    }
}

void HttpReq::parseHeader() {
	if (!is_header_end) {
		return;
	}
	skipEmptyLines();
	if (getErrStatus() > 0) {
		return ;
	}
    parseReqLine();
	if (getErrStatus() > 0) {
		return;
	}
	while (idx < buf.size()) {
		if (checkHeaderEnd()) {
			break;
		}
		std::string field_name = getToken(':');
		if (getErrStatus() > 0) {
			std::cout << "return (" << err_status << std::endl;
			return;
		}
		skipSpace();
		std::string field_value = getTokenToEOL();
		trim(field_value);
		checkVCHAR(field_value);
		setHeaderField(toLower(field_name), field_value);
		if (getErrStatus() > 0) {
			std::cout << "return2 (" << err_status << std::endl;
			return;
		}
	}
	fixUp();
	// \r\n\r\nと一緒にbodyも全て送られてきた場合ここで判定しないといけない
	if (getHeaderFields().count("content-length") == 1 && content_length <= body_buf.size())
		is_req_end= true;
	idx = 0;
}

void HttpReq::parseBody() {
	if (header_fields.count("transfer-encoding") == 1 && header_fields["transfer-encoding"] == "chunked") {
		parseChunk();
	} else if (header_fields.count("content-length") == 1) {
		if (header_fields.size() > 0 && content_length <= body_buf.size()) {
			content_body += body_buf.substr(0, content_length);
			std::cout << "content_body_buf: " << body_buf << std::endl;
			is_req_end = true;
		} else {
			return;
		}
	} else {
		is_req_end = true;
		return;
	}
}

std::map<std::string, std::string> HttpReq::get_meta_variables() const {
    return cgi_envs;
}

std::string HttpReq::percentEncode() {
	std::ostringstream rets;
	for(size_t n = 0; n < query_string.size(); n++) {
	  unsigned char c = (unsigned char)query_string[n];
	  if (isalnum(c) || c == '+' || c == '&' || c == '=' )
	    rets << c;
	  else {
		rets << '%' << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << int(c);
	  }
	}
	return rets.str();
}

void HttpReq::set_meta_variables(Location loc) {
    std::map<std::string, std::string> header_fields = getHeaderFields();
    if (header_fields.count("content-length") != 0) {
        cgi_envs["CONTENT_LENGTH"] = header_fields["content-length"];
    }
    if (header_fields.count("content-type") != 0) {
        cgi_envs["CONTENT_TYPE"] = header_fields["content-type"];
    }
    cgi_envs["GATEWAY_INTERFACE"] = "CGI/1.1";
	// Locationで取得したcgi拡張子とマッチするものがあるときにPATH_INFOを区切る
	std::vector<std::string> ext = loc.getCgiExt();
	for (std::vector<std::string>::iterator it = ext.begin(); it != ext.end(); it++) {
		std::string::size_type idx = uri.find(*it);
		size_t len = (*it).size();
		if (idx == std::string::npos)
			continue;
		if ((uri[idx + len] != '\0' && uri[idx + len] == '/') || uri[idx + len] == '\0') {
			std::cout << "SET_SCRIPT_NAME" << std::endl;
			cgi_envs["SCRIPT_NAME"] = uri.substr(0, idx + len);
			cgi_envs["PATH_INFO"] = uri.substr(idx + len);
			if (cgi_envs["PATH_INFO"] != "")
				cgi_envs["PATH_TRANSLATED"] = loc.getRoot() + cgi_envs["PATH_INFO"];
			else
				cgi_envs["PATH_TRANSLATED"] = "";
		}
	}
	cgi_envs["QUERY_STRING"] = percentEncode();
	std::cout << "envs: " << cgi_envs["QUERY_STRING"] << std::endl;
    cgi_envs["REMOTE_ADDR"] = getClientIP();
    cgi_envs["REMOTE_HOST"] = cgi_envs["REMOTE_ADDR"];
	cgi_envs["REQUEST_METHOD"] = getMethod();
    cgi_envs["SERVER_NAME"] = header_fields["host"];
    std::stringstream ss;
    std::string port_str;
    ss << getPort();
    ss >> port_str;
    cgi_envs["SERVER_PORT"] = port_str;
    cgi_envs["SERVER_PROTOCOL"] = "HTTP/1.1";
    cgi_envs["SERVER_SOFTWARE"] = "WebServe";
}

std::ostream& operator<<(std::ostream& stream, const HttpReq& obj) {
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

