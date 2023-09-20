#include "HttpRes.hpp"
#include "../Client.hpp"

const std::string HttpRes::kServerName = "WebServe";
const std::string HttpRes::default_type = "text/html";

std::string getContentType(std::string type) {
	if (type == "html" || type == "csv" || type == "css" || type == "js")
		return "text/" + type;
    else if (type == "txt")
        return "text/plain";
	else if (type == "json" || type == "pdf" || type == "zip")
		return "application/" + type;
	else if (type == "png" || type == "webp" || type == "gif")
		return "image/" + type;
    else if (type == "jpeg" || type == "jpg")
        return "image/jpeg";
    else if (type == "wav")
        return "audio/" + type;

	return "";
}

std::string getContentExtension(std::string content_type) {
    std::cout << "ct: " << content_type << std::endl;
    if (content_type == "text/html") {
        return ".html";
    } else if (content_type == "text/plain") {
        return ".txt";
    } else if (content_type == "text/csv") {
        return ".csv";
    } else if (content_type == "text/css") {
        return ".css";
    } else if (content_type == "text/js") {
        return ".js";
    } else if (content_type == "application/json") {
        return ".json";
    } else if (content_type == "application/pdf") {
        return ".pdf";
    } else if (content_type == "application/zip") {
        return ".zip";
    } else if (content_type == "image/png") {
        return ".png";
    } else if (content_type == "image/jpeg" || content_type == "image/jpg") {
        return ".jpeg";
    } else if (content_type == "image/webp") {
        return ".webp";
    } else if (content_type == "image/gif") {
        return ".gif";
    } else if (content_type == "audio/wav") {
        return ".wav";
    } else {
        return "";
    }
}

HttpRes::HttpRes() {
}

HttpRes::HttpRes(const Client& source, Kqueue &kq)
:content_length_n(0),
    is_posted(0),
    err_status(0),
    is_sended_header(false),
    is_sended_body(false)
{
	this->httpreq = source.getHttpReq();
	this->vServer = source.getVserver();
    this->connection = &kq;
	this->fd = source.getFd();
	this->keep_alive = httpreq.getKeepAlive();
}

HttpRes::HttpRes(const HttpRes& src) {
    this->buf = src.buf;
    this->header_size = src.header_size;
    this->out_buf = src.out_buf;
    this->body_size = src.body_size;
    this->is_sended_header = src.getIsSendedHeader();
    this->is_sended_body = src.getIsSendedBody();
    this->connection = src.getConnection();
	this->keep_alive = src.keep_alive;

}

HttpRes& HttpRes::operator=(const HttpRes& rhs) {
	if (this == &rhs) {
		return *this;
	}
    this->buf = rhs.buf;
    this->header_size = rhs.header_size;
    this->out_buf = rhs.out_buf;
    this->body_size = rhs.body_size;
    this->is_sended_header = rhs.getIsSendedHeader();
    this->is_sended_body = rhs.getIsSendedBody();
    this->connection = rhs.getConnection();
	this->keep_alive = rhs.keep_alive;
	return *this;
}

HttpRes::~HttpRes() {
}


void HttpRes::setIsSendedHeader(bool b) {
	this->is_sended_header = b;
}

void HttpRes::setIsSendedBody(bool b) {
	this->is_sended_body = b;
}

bool HttpRes::getIsSendedBody() const {
	return is_sended_body;
}

bool HttpRes::getIsSendedHeader() const {
	return is_sended_header;
}


std::string HttpRes::getBuf() const {
    return buf;
}

size_t HttpRes:: getHeaderSize() const {
    return header_size;
}

std::string HttpRes::getResBody() const {
    return out_buf;
}

size_t HttpRes::getBodySize() const {
    return body_size;
}


void HttpRes::setLocationField(std::string loc) {
    this->location_field = loc;
}

std::string HttpRes::getLocationField() const {
    return location_field;
}

int HttpRes::getKeepAlive() const {
	return keep_alive;
}

bool HttpRes::isHeaderOnly() const {
	return header_only;
}

Kqueue* HttpRes::getConnection() const {
    return connection;
}

Location HttpRes::getUri2Location(std::string uri) const
{
	std::string tmp_uri = uri;
	if (tmp_uri != "" && tmp_uri[tmp_uri.length() - 1] != '/') {
		tmp_uri = tmp_uri + '/';
	}
	std::map<std::string, Location> uri2location = vServer.getUri2Location();
	std::map<std::string, Location>::const_iterator loc = uri2location.find(tmp_uri);
	if (loc != uri2location.end()) {
		return loc->second;
	}
	std::string path = uri;
	while (1) {
		loc = uri2location.find(path);
		if (loc != uri2location.end()) {
			return loc->second;
		}
		std::string::size_type i = path.rfind('/');
		if (i == std::string::npos) {
			break;
		}
		if (i == 0) {
			path = path.substr(0, 1);
        } else {
		    path = path.substr(0, i + 1);
        }
		loc = uri2location.find(path);
		if (loc != uri2location.end()) {
			return loc->second;
		} else {
			path = path.substr(0, i);
		}
	}
    Location no_match_loc;
    return no_match_loc;
}

void HttpRes::createErrorResponse(int status) {
	status_code = status;
	headerFilter();
	out_buf = "";
}

Location HttpRes::longestMatchLocation(std::string request_path, std::vector<Location> locations) {
	Location location;
	size_t max_len = 0;
	for (std::vector<Location>::iterator it = locations.begin(); it != locations.end(); it++) {
        std::string location_path = it->getUri();
		if (request_path.find(location_path) == 0) {
			if (request_path[location_path.length()] == '/' || request_path.length() == location_path.length()) {
				if (location_path.length() > max_len) {
					max_len = location_path.length();
					location = *it;
				}
			}
		}
	}
	return location;
}

bool HttpRes::isAllowMethod(std::string method) {
	std::vector<std::string> allow_method = target.get_methods();
	for (std::vector<std::string>::iterator it = allow_method.begin();
		it != allow_method.end(); it++) {
		if (*it == method) {
			return true;
		}
	}
	return false;
}

std::string HttpRes::joinPath() {
//    std::cout << "===== joinPath =====" << std::endl;
	std::string path_root = target.getRoot();
	if (path_root == "" || path_root[0] != '/') {
		path_root = "./" + path_root;
	}
	std::string config_path  = target.getUri();
    std::string upload_path = target.getUploadPath();
	std::string file_path = httpreq.getUri();
	std::cout << file_path << std::endl;
	if (config_path.length() < file_path.length()) {
		file_path = file_path.substr(config_path.length());
	} else {
		file_path = "";
	}

	std::cout << path_root << std::endl;
	std::cout << config_path << std::endl;
    std::cout << upload_path << std::endl;
	std::cout << file_path << std::endl;


    std::string method = httpreq.getMethod();
    if (method != "POST") {
        upload_path = "";
    }
    int index_flag = 0;
	if ((file_path[file_path.length() -1 ] == '/' || file_path == "") && config_path[config_path.length() - 1] == '/' && method != "POST") {
        index_flag = 1;
	}
	std::string alias;
	if ((alias = target.getAlias()) != "") {
		config_path = "";
        path_root = alias;
	}
    if ((upload_path != "" && method == "POST" && path_root.size() && path_root[path_root.size() - 1] == '/')
        || (upload_path != "" && method == "POST" && path_root.size() == 0)) {
            upload_path = upload_path.substr(1);
    }
    else if ((path_root.size() && path_root[path_root.length() - 1] == '/') || path_root.size() == 0) {
		if (config_path.size() >= 1)
			config_path = config_path.substr(1);
    }
    if (index_flag) {
        std::vector<std::string> index_files = target.getIndex();
        if (index_files.size() != 0) {
            for (std::vector<std::string>::iterator it = index_files.begin(); it != index_files.end(); ++it) {
                std::string full_path = path_root + config_path + file_path + *it;
                if (access(full_path.c_str(), R_OK) >= 0) {
                    std::cout << "full_path: " << full_path << std::endl;
                    return full_path;
                }
            }
        } else {
            std::cout << "no index directive: " << path_root + config_path + file_path + "index.html" << std::endl;
            return path_root + config_path + file_path + "index.html";
        }
        std::cout << "no macth index: " << path_root + config_path + file_path + *(index_files.begin()) << std::endl;
        return path_root + config_path + file_path + *(index_files.begin());
    }
	std::cout << "join_path: " << path_root + upload_path + config_path + file_path << std::endl;
	return path_root + upload_path + config_path + file_path;
}

void HttpRes::setBody(std::string strs)
{
    this->body = strs;
}

void HttpRes::setCgi(Cgi cgi) {
    this->cgi = cgi;
}

Cgi HttpRes::getCgi() const {
    return cgi;
}

//std::string HttpRes::getStatusString() {
//	switch (status_code) {
//		case 200:
//			return "OK\n";
//		case 404:
//			return "Not Found\n";
//	}
//	return "Error(statusString)\n";
//}
//
//void HttpRes::createControlData() {
//	header += "HTTP1.1 ";
//	std::stringstream ss;
//	ss << status_code;
//	header += ss.str();
//	header += " ";
//	header += getStatusString();
//}

//std::string HttpRes::createDate(time_t now, std::string fieldName)
std::string HttpRes::createDate(std::string fieldName)
{
	time_t now = std::time(NULL);
    std::string str;
    char buf[1000];
    struct tm tm = *gmtime(&now);
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S ", &tm);
	str += fieldName + ": ";
    std::string date(buf);
    str += date + "GMT\n";
    return str;
}

void HttpRes::createContentLength() {
	std::stringstream ss;
	std::string code;
	ss << body.length();
	header += "Content-Length: ";
	header += ss.str();
}

int HttpRes::setContentType() {
	std::string ext;
	std::string type;
	std::string uri = httpreq.getUri();
	std::string::size_type dot_pos = uri.rfind('.');

	if (dot_pos != std::string::npos) {
		ext = uri.substr(dot_pos + 1);
		if (ext.length() == 0) {
			std::cerr << "Error dot" << std::endl;
			status_code = BAD_REQUEST;
			return status_code;
		}
	}
	for (size_t i = 0; i < ext.length(); i++) {
		if (ext[i] >= 'A' && ext[i] <= 'Z') {
			type += std::tolower(ext[i]);
		} else {
			type += ext[i];
		}
	}

	content_type = getContentType(type);

    if (content_type.length() == 0) {
	    content_type = default_type;
    }
	return status_code;
}

/*
void HttpRes::evQueueInsert() {
	std::cout << "connection: " << connection << std::endl;
	connection->setEvent(fd, EVFILT_WRITE, EV_ENABLE);
    std::cout << "==================send write event==================" << std::endl;
}

void HttpRes::postEvent() {
    if (!is_posted) {
        is_posted = 1;
        evQueueInsert();
    }
}
*/

std::map<int, std::string> create_status_msg(){
    std::map<int, std::string> m;
    m[200] = "OK";
    m[201] = "Created";
    m[202] = "Accepted";
    m[203] = "";
    m[204] = "No Content";
    m[205] = "";
    m[206] = "Partial Content";
    m[301] = "Moved Permanently";
    m[302] = "Moved Temporarily";
    m[303] = "See Other";
    m[304] = "Not Modified";
    m[307] = "Temporary Redirect";
    m[308] = "Permanent Redirect";

    m[400] = "Bad Request";
    m[401] = "Unauthorized";
    m[402] = "Payment Required";
    m[403] = "Forbidden";
    m[404] = "Not Found";
    m[405] = "Not Allowed";
    m[406] = "Not Acceptable";
    m[408] = "Request Time-out";
    m[409] = "Conflict";
    m[410] = "Gone";
    m[411] = "Length Required";
    m[412] = "Precondition Failed";
    m[413] = "Request Entity Too Large" ;
    m[414] = "Request-URI Too Large";
    m[415] = "Unsupported Media Type";
    m[416] = "Requested Range Not Satisfiable";
    m[421] = "Misdirected Request";
    m[429] = "Too Many Requests";

    m[500] = "Internal Server Error";
    m[501] = "Not Implemented";
    m[502] = "Bad Gateway";
    m[503] = "Service Temporarily Unavailable";
    m[504] = "Gateway Time-out";
    m[505] = "HTTP Version Not Supported";
    m[507] = "Insufficient Storage";
	return m;
}

int HttpRes::dav_depth() {
	int depth = target.getDepth();
	return depth;
}



std::string HttpRes::joinDirPath(const std::string& dir_path, const std::string& elem_name) {
	return dir_path + '/' + elem_name;
}

void HttpRes::divingThroughDir(const std::string& path) {
    dir_t dir_info;
    dir_info.dir = opendir(path.c_str());
    if (dir_info.dir == NULL) {
        std::cerr << "opendir Error" << std::endl;
		status_code = INTERNAL_SERVER_ERROR;
		return;
    }
    dir_info.valid_info = 0;
    for (;;) {
        errno = 0;
        dir_info.d_ent = readdir(dir_info.dir);
        if (!dir_info.d_ent) {
            if (errno != 0) {
				std::cerr << "readdir Error" << std::endl;
				status_code = INTERNAL_SERVER_ERROR;
            }
            if (closedir(dir_info.dir) == -1) {
                std::cerr << "closedir Error" << std::endl;
            }
            return;
        }
        std::string file_name(dir_info.d_ent->d_name);
        if (file_name.length() == 1 && file_name[0] == '.') {
            continue;
        }
        if (file_name.length() == 2 && file_name[0] == '.' && file_name[1] == '.') {
            continue;
        }
        std::string abs_path = joinDirPath(path, file_name);

        if (!dir_info.valid_info) {
            if (stat(abs_path.c_str(), &(dir_info.d_info)) == -1) {
                continue;
            }
        }
        if ((S_ISREG(dir_info.d_info.st_mode))) {
            if (remove(abs_path.c_str()) < 0) {
                std::cerr << "remove Error" << std::endl;
                status_code = INTERNAL_SERVER_ERROR;
                if (closedir(dir_info.dir) == -1) {
                    std::cerr << "closedir Error" << std::endl;
                }
                return;
            }
        } else if ((S_ISDIR(dir_info.d_info.st_mode))) {
            divingThroughDir(abs_path);
            if (status_code == INTERNAL_SERVER_ERROR) {
                if (closedir(dir_info.dir) == -1) {
                    std::cerr << "closedir Error" << std::endl;
                }
                return;
            }
            if (rmdir(abs_path.c_str()) < 0) {
                std::cerr << "rmdir Error" << std::endl;
                status_code = INTERNAL_SERVER_ERROR;
                if (closedir(dir_info.dir) == -1) {
                    std::cerr << "closedir Error" << std::endl;
                }
                return;
            }
        } else {
            if (remove(abs_path.c_str()) < 0) {
                std::cerr << "remove Error" << std::endl;
                status_code = INTERNAL_SERVER_ERROR;
                if (closedir(dir_info.dir) == -1) {
                    std::cerr << "closedir Error" << std::endl;
                }
                return;
            }
      }
    }
}

int HttpRes::deleteError() {
    if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG) {
		status_code = NOT_FOUND;
        return NOT_FOUND;
    } else if (errno == EACCES || errno == EPERM) {
		status_code = FORBIDDEN;
        return FORBIDDEN;
    } else {
		status_code = INTERNAL_SERVER_ERROR;
        return INTERNAL_SERVER_ERROR;
    }
}

int HttpRes::deletePath(bool is_dir) {
    if (is_dir) {
        std::string dir_path = joinPath();
        divingThroughDir(dir_path);
        if (status_code == INTERNAL_SERVER_ERROR) {
            return status_code;
        }
        if (rmdir(dir_path.c_str()) >= 0) {
            status_code = NO_CONTENT;
			header_only = 1;
            return status_code;
        }

	} else {
		std::string file_name = joinPath();
		if (remove(file_name.c_str()) >= 0) {
		    status_code = NO_CONTENT;
			header_only = 1;
			return status_code;
		}
	}
    return deleteError();
}


int HttpRes::deleteHandler() {
	std::cout << "====================dav delete handler====================" << std::endl;
	int content_length = httpreq.getContentLength();
	if (content_length > 0) {
		status_code = UNSUPPORTED_MEDIA_TYPE;
	}

	struct stat sb;
	bool is_dir;
	int depth;
	std::string method = httpreq.getMethod();
	if (method != "DELETE") {
		return DECLINED;
	}

	std::vector<std::string> allow_methods = target.get_methods();
	if (find(allow_methods.begin(), allow_methods.end(), method) == allow_methods.end()) {
		std::cout << "not allow (conf)" << std::endl;
		status_code = BAD_REQUEST;
		return status_code;
	}

	std::string file_name = joinPath();
    if (stat(file_name.c_str(), &sb) == -1) {
		std::cout << "Error(stat)" << std::endl;
		status_code = NOT_FOUND;
		return status_code;
	}
	if (S_ISDIR(sb.st_mode)) {
		std::string uri = httpreq.getUri();
		depth = dav_depth();
		if (depth != -1) {
			status_code = BAD_REQUEST;
			return status_code;
		}
		is_dir = true;
	} else {
		depth = dav_depth();
		if (depth != 0 && depth != -1) {
			status_code = BAD_REQUEST;
			return status_code;
		}
		is_dir = false;
	}
	return deletePath(is_dir);
}

void HttpRes::createStatusLine() {
	std::map<int, std::string> status_msg = create_status_msg();
	if (last_modified_time != -1) {
		if (status_code != HTTP_OK) {
			last_modified_time = -1;
		}
	}

	if (status_line == "") {
        std::stringstream ss;
        ss << status_code;
        std::string status_code_str = ss.str();
        ss >> status_code_str;
		if (status_code >= INTERNAL_SERVER_ERROR) {
            // 5XX
            status_line = "HTTP/1.1 " + status_code_str + ' ' + status_msg[status_code];
		} else if (status_code >= BAD_REQUEST) {
            // 4XX
            status_line = "HTTP/1.1 " + status_code_str + ' ' +  status_msg[status_code];
		}
        else if (status_code >= MOVED_PERMANENTLY) {
            // 3XX
            if (status_code == NOT_MODIFIED) {
                header_only = 1;
            }
            status_line = "HTTP/1.1 " + status_code_str + ' ' +  status_msg[status_code];
		} else if (status_code >= HTTP_OK) {
            // 2XX
            if (status_code == NOT_MODIFIED) {
                header_only = 1;
            }
            if (status_code == NO_CONTENT) {
//                header_only = 1;
                content_type = "";
                last_modified_time = NULL;
            }
            status_line = "HTTP/1.1 " + status_code_str + ' ' +  status_msg[status_code];
        } else {
            //
			status_line = "";
		}
	}
	buf += status_line;
	buf += "\r\n";
}

void HttpRes::addAllowField() {
    std::vector<std::string> allow_methods = target.get_methods();
    buf += "Allow: ";
    for (std::vector<std::string>::iterator it = allow_methods.begin(); it != allow_methods.end(); ++it) {
        buf += *it + ' ';
    }
    buf += "\r\n";
}

void HttpRes::addContentTypeField() {
	buf += "Content-Type: " + content_type;

	if (charset != "") {
		buf += "; charset=" + charset;

		// content_type に charsetを加える
	}
	buf += "\r\n";
}

void HttpRes::addContentLengthField() {
    std::stringstream ss;
    ss << content_length_n;
    buf += "Content-Length: " + ss.str();
    buf += "\r\n";
//	} else {
//	buf += "Content-Length: 0";
//	buf += "\r\n";
}

void HttpRes::addConnectionField() {
	std::cout << "keep-alive in header: " << keep_alive << std::endl;
    if (this->keep_alive) {
        buf += "Connection: keep-alive";
    } else {
	    buf += "Connection: close";
    }
	buf += "\r\n";
}

void HttpRes::addLocationField() {
	if (status_code == 201 && getLocationField() != "") {
		std::string loc_field_value = getLocationField();
		if (loc_field_value[0] == '.') {
			loc_field_value = loc_field_value.substr(1);
		}
        buf += "Location: " + loc_field_value;
//        buf += "Location: " + getLocationField();
        buf += "\r\n";
    }
	else if (status_code >= 300 && status_code < 400 && redirect_path.length()> 0) {
		buf += "Location: " + redirect_path;
		buf += "\r\n";
	}
}

void HttpRes::headerFilter() {
	createStatusLine();
    if (status_code == NOT_ALLOWED) {
		addAllowField();
    }
	buf += "Server: " + kServerName; //matchしたserve_nameに変更
	buf += "\r\n";

    buf += createDate("Date");

	if (content_type != "") {
		addContentTypeField();
	} if (content_length_n > 0) {
		addContentLengthField();
//    } else {
//		buf += "Content-Length: 0";
//	    buf += "\r\n";
	}
	if (last_modified_time != -1) {
		//buf += "Last-Modified: " + http_time();
//		buf += "\r\n";
	}
	std::map<std::string, std::string> cgi_headers = cgi.getHeaderFields();
	std::map<std::string, std::string>::iterator it= cgi_headers.begin();
	for (; it != cgi_headers.end(); ++it) {
		if (it->first == "Location")
			continue;
        std::cout << it->first << ": " << it->second << std::endl;
		buf += it->first;
		buf += ": ";
		buf += it->second;
		buf += "\r\n";
	}

//    if (httpreq.getKeepAlive()) {
	addConnectionField();
	addLocationField();
	// 残りのヘッダー  もしかしたら必要ないかも？ 現状Connection filedなどがダブってしまっているetc...
	//std::map<std::string, std::string> headers = httpreq.getHeaderFields();
//	std::map<std::string, std::string>::iterator it= headers.begin();
//	for (; it != headers.end(); it++) {
//		buf += it->first;
//		buf += ": ";
//		buf += it->second;
//		buf += "\r\n";
//	}
	buf += "\r\n";
	header_size = buf.size();

	//
    std::cout << "response Header: " << std::endl;
	std::cout << buf << std::endl;
    //postEvent();
}

void HttpRes::sendHeader() {
    // check alredy sent
    if (err_status) {
        status_code = err_status;
    }
    return headerFilter();
}

int HttpRes::checkAccessToGET(const char *file_name, const std::string& uri) { //or safe method
	std::cout << file_name << std::endl;
    if (access(file_name, R_OK) < 0) {
        std::cerr << "open Error" << std::endl;
		std::cout << "errno: " << errno << std::endl;
        if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG) {
            if (target.getIsAutoindex() && uri[uri.length() - 1] == '/') {
                return DECLINED;
            } else if (target.getIndex().size() > 0 && uri[uri.length() - 1] == '/') {
                std::cout << "FORBIDDEN1" << std::endl;
				std::cout << location << std::endl;
                std::cout << uri << std::endl;
//                    status_code = FORBIDDEN;
				status_code = NOT_FOUND;
                return FORBIDDEN;
            }
            std::cout << "NOT FOUND" << std::endl;
            status_code = NOT_FOUND;
            return NOT_FOUND;
        } else if (errno == EACCES){
            std::cout << "FORBIDDEN2" << std::endl;
//                status_code = FORBIDDEN;
			status_code = NOT_FOUND;
            return FORBIDDEN;
        }
		std::cout << "errno: " << errno << std::endl;
        status_code = INTERNAL_SERVER_ERROR;
        return INTERNAL_SERVER_ERROR;
    }
	return OK;
}

int HttpRes::HandleSafeMethod(const char *file_name, std::string& uri) {
	int handler_status = checkAccessToGET(file_name, uri);
	if (handler_status != OK) {
		return handler_status;
	}
    struct stat sb;
    if (stat(file_name, &sb) == -1) {
        std::cout << "GET Error(stat)" << std::endl;
        status_code = INTERNAL_SERVER_ERROR;
        return status_code;
    }
    if (S_ISDIR(sb.st_mode)) {
        uri.push_back('/');
        httpreq.setUri(uri);
        if (target.getIndex().size() > 0 || target.getIsAutoindex()) {
            return staticHandler();
        } else {
            return DECLINED;
        }
    } else if (!S_ISREG(sb.st_mode)) {
        std::cerr << "NOT FOUND(404)" << std::endl;
        status_code = NOT_FOUND;
        return NOT_FOUND;
    }
	content_length_n = sb.st_size;
	if (content_length_n == 0) {
		status_code = NO_CONTENT;
		header_only = 1;
	}
	last_modified_time = sb.st_mtime;
	return status_code;
}

int HttpRes::checkAccessToPOST(const char *file_name) {
    if (access(file_name, W_OK) < 0) {
		std::cerr << "POST open Error" << std::endl;
		if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG) {
			std::cout << "NOT FOUND" << std::endl;
			status_code = NOT_FOUND;
			return NOT_FOUND;
		} else if (EACCES){
			std::cout << "FORBIDDEN" << std::endl;
//				  status_code = FORBIDDEN;
			status_code = NOT_FOUND;
			return FORBIDDEN;
		}
		status_code = INTERNAL_SERVER_ERROR;
		return INTERNAL_SERVER_ERROR;
	}
	return OK;
}

int HttpRes::createDestFile(std::string& file_name) {
    time_t tm = std::time(NULL);
    std::stringstream ss;
    ss << tm;
    std::string ext = getContentExtension(httpreq.getHeaderFields()["content-type"]);
    if (file_name[file_name.length() - 1] != '/') {
        file_name = file_name + '/' + ss.str() + ext;
    } else {
        file_name = file_name + ss.str() + ext;
    }
    std::ofstream tmp_ofs(file_name);
    if (tmp_ofs.bad()) {
        status_code = INTERNAL_SERVER_ERROR;
        return INTERNAL_SERVER_ERROR;
    }
    tmp_ofs.close();
	if (checkAccessToPOST(file_name.c_str()) != OK) {
		return status_code;
	}
	return OK;
}

int HttpRes::handlePost(std::string& file_name) {
    struct stat sb;
    if (stat(file_name.c_str(), &sb) == -1) {
		std::cout << "errno: " << errno << std::endl;
		if (errno == ENOENT) {
		    status_code = CREATED;
            setLocationField(file_name);
			std::ofstream tmp_ofs(file_name);
			if (tmp_ofs.bad()) {
				status_code = INTERNAL_SERVER_ERROR;
				return INTERNAL_SERVER_ERROR;
			}
			tmp_ofs.close();
		} else if (errno == ENOTDIR || errno == ENAMETOOLONG) {
            status_code = NOT_FOUND;
            return status_code;
        } else {
//                std::cout << "POST errno: " << errno << std::endl;
        }
    } else {
        status_code = HTTP_OK;
        std::string ext = getContentExtension(httpreq.getHeaderFields()["content-type"]);
			// 対応していない拡張子かつcontent-typeが存在する場合
		if (ext == "" && httpreq.getHeaderFields()["content-type"] != "") {
			status_code = UNSUPPORTED_MEDIA_TYPE;
			return status_code;
		}
    }
    if (S_ISDIR(sb.st_mode)) {
		if (createDestFile(file_name) != OK) {
			return status_code;
		}
        status_code = CREATED;
        setLocationField(file_name);
    }
    content_length_n = sb.st_size;
//	if (content_length_n == 0) {
//		status_code = NO_CONTENT;
//		header_only = 1;
//	}
	last_modified_time = sb.st_mtime;
    if (!S_ISREG(sb.st_mode) && status_code != CREATED) { // neccessary?
		std::cerr << "stat Error" << std::endl;
        return INTERNAL_SERVER_ERROR;
    } else {
		if (checkAccessToPOST(file_name.c_str()) != OK) {
			return status_code;
		}
        std::ofstream ofs(file_name.c_str(), std::ios::app);
		if (!ofs) {
            std::cerr << "POST open Error" << std::endl;
            status_code = INTERNAL_SERVER_ERROR;
            return INTERNAL_SERVER_ERROR;
		}
		std::cout << "set body done" << std::endl;
		std::string body = httpreq.getContentBody();
		if (body.length() == 0 && content_length_n == 0 && status_code != CREATED) {
			status_code = NO_CONTENT;
			header_only = 1;
		}
		std::cout << body << std::endl;
        ofs << body;
        ofs.close();
        //content_length_n = body.size();
	}
	return OK;
    //discoard request body here ?
}

int HttpRes::handleResBody(const std::string& file_name) {
    if (!header_only) {
		std::ifstream ifs(file_name.c_str(), std::ios::binary);
		if (!ifs) {
			std::cout << "file_name: " << file_name << std::endl;
			status_code = INTERNAL_SERVER_ERROR;
			return status_code;
		}
        std::ostringstream oss;
        oss << ifs.rdbuf();
        out_buf = oss.str();
		std::cout << out_buf << std::endl;
		content_length_n = out_buf.length();
		std::cout << content_length_n << std::endl;
//        body_size = content_length_n;
		body_size = out_buf.length();
		return OK;
    }
	return OK;

}

int HttpRes::staticHandler() {
	std::cout << "================== staticHandler ==================" << std::endl;
	std::string uri = httpreq.getUri();
    std::cout << "uri: " << uri << std::endl;
	target = getUri2Location(uri);
    if (target.getUri() == "") {
        status_code = NOT_FOUND;
        return status_code;
    }
	std::cout << target << std::endl;
	std::string method = httpreq.getMethod();
	if (method != "GET" && method != "HEAD" && method != "POST" && method != "PUT") {
        std::cerr << "not allow method in static handler" << std::endl;
		return DECLINED;
	}
	std::vector<std::string> allow_methods = target.get_methods();
	if (find(allow_methods.begin(), allow_methods.end(), method) == allow_methods.end()) {
        status_code = NOT_ALLOWED;
		return status_code;
	}
	std::string file_name = joinPath();
	int handler_status;
    status_code = HTTP_OK;
    if (method == "GET" || method == "HEAD") {
		handler_status = HandleSafeMethod(file_name.c_str(), uri);
		if (handler_status >= 300) {
			return status_code;
		} else if (handler_status == DECLINED) {
			return DECLINED;

		}
    } else if (method == "POST" || method == "PUT") {
		if (handlePost(file_name) != OK) {
			return status_code;
		}
    }
    //discoard request body here ?
	handler_status = setContentType();
	if (handler_status == BAD_REQUEST) {
		return status_code;
	}
	if (handleResBody(file_name) != OK) {
		return status_code;
	}
    sendHeader();
    return OK;
}

std::string HttpRes::createErrPage() {
    std::map<int, std::string> status_msg_map = create_status_msg();
//    std::string err_page_buf = "<!DOCTYPE html>" "\r\n";
    std::string err_page_buf = "<html>" "\r\n""<head><title>";
    std::stringstream ss;
	ss << status_code;
	err_page_buf += ss.str();
    err_page_buf += " ";
    err_page_buf += status_msg_map[status_code];
    err_page_buf += "</title></head>" "\r\n""<body>" "\r\n""<center><h1>";
	err_page_buf += ss.str();
    err_page_buf += " ";
    err_page_buf += status_msg_map[status_code];
    err_page_buf += "</h1></center>" "\r\n";
    err_page_buf += "<hr><center>";
    err_page_buf += kServerName;
    err_page_buf += "</center>" "\r\n""</body>""\r\n""</html>";

    std::cout << "err_page: " << err_page_buf << std::endl;
    return err_page_buf;
}

int HttpRes::sendErrorPage() {
    std::string path = target.getErrorPage(status_code);
    if (path[0] == '/') {
        std::string method = httpreq.getMethod();
        if (method != "HEAD") { //we non-supported HEAD
            method = "GET";
        }
        httpreq.setUri(path);
        if (buf.length()) {
            buf.erase();
        }
        if (out_buf.length()) {
            out_buf.erase();
        }
        runHandlers();
        return OK;
    }
    //if path[0] == '@' non-supported
    //  nameed_location
	return 0;
}

int HttpRes::redirectHandle() {
    std::cout << "================== redirectHandle ==================" << std::endl;
    err_status = status_code;
    switch (status_code) {
        case BAD_REQUEST:
        case REQUEST_ENTITY_TOO_LARGE:
		case REQUEST_TIME_OUT:
//        case REQUEST_URI_TOO_LARGE:
//        case HTTP_TO_HTTPS:
//        case HTTPS_CERT_ERROR:
//        case HTTPS_NO_CERT:
        case INTERNAL_SERVER_ERROR:
        case HTTP_NOT_IMPLEMENTED:
            keep_alive = 0;
    }
    content_type.erase();

    if (target.getErrorPage(status_code) != "") {
        return sendErrorPage();
    }
//     discard request body
    if (out_buf.length()) {
        out_buf.erase();
        content_length_n = 0;
    }

//    if (status_code >= 490) { //49x ~ 5xx
//        switch (status_code) {
//            case HTTP_TO_HTTPS:
//            case HTTPS_CERT_ERROR:
//            case HTTPS_NO_CERT:
//            case HTTP_REQUEST_HEADER_TOO_LARGE:
//                status_code = BAD_REQUEST;
                // or err_status = BAD_REQUEST;
//        }
//    } else {
//        std::cout << "unknown status code" << std::endl;
//    }
    // if We create a new file, how do We handle mtime?
    std::string err_page_buf = std::string();
    if (status_code >= 300) {
        err_page_buf = createErrPage();
    }
    if (err_page_buf.length()) {
        content_length_n = err_page_buf.length();
        content_type = "text/html";
    }
    else {
        content_length_n = 0;
    }
//  clear accept_range
//  clear last_modified
    last_modified_time = -1;
//  clear etag
    sendHeader();
//    if err || only_header
//        return
//    if content_length == 0
        // something
	if (!header_only) {
		out_buf = err_page_buf;
		body_size = content_length_n;
	}
    return OK;
}

void HttpRes::handleReqErr(int req_err_status) {
    status_code = req_err_status;
    finalizeRes(req_err_status);
}


void HttpRes::finalizeRes(int handler_status)
{
	std::cout << "================== finalizeRes ==================" << std::endl;
    if (handler_status == DECLINED || handler_status == OK) {
        return;
    }
    if ((200 <= status_code && status_code < 207) && status_code != 204) {
        // handle connection
        return;
    }
    if (status_code >= 300 || status_code == 204) {
        // handle around timeer
        redirectHandle();
        return;
    }
}

int HttpRes::returnRedirect() {
	std::cout << "================== return redirect ==================" << std::endl;
	std::string uri = httpreq.getUri();
	Location loc = getUri2Location(uri);
	std::string ret = loc.getReturn();
	std::cout << "loc: " << loc << std::endl;
	std::cout << "ret: " << ret << std::endl;
	if (ret == "")
		return DECLINED;
	std::vector<std::string> elms;
	std::string elm;
	std::stringstream ss1(ret);

	while (std::getline(ss1, elm, ' ')) {
		if (!elm.empty()) {
			elms.push_back(elm);
		}
	}
	std::stringstream ss(elms[0]);
	ss >> status_code;
	std::string path;
	if (ss.fail()) {
		path = elms[0];
		if (!path.compare(0, 7, "http://") || !path.compare(0, 8, "https://")) {
			status_code = MOVED_TEMPORARILY;
//            return status_code;
			//path = elms[0];
		} else {
			std::cout << "scheme Error" << std::endl;
			return DECLINED; //ERROR;
		}
	} else {
		if (status_code > 999) {
			std::cout << "status_code Error" << std::endl;
			return DECLINED; //ERROR;
		}
		if (elms.size() == 1) {
			std::cout << "status_code only" << std::endl;
			return DECLINED; //OK;
		}
		path = elms[1];
	}
	redirect_path = path;
	std::cout << "redirect_path: " << redirect_path << std::endl;
    // needs path with support status_code
	// compile_complex_valueは$の展開をしてそう
    return status_code;
}

static std::string createMtime(time_t modified)
{
    char buf[1000];
    struct tm tm = *std::gmtime(&modified);
    std::strftime(buf, sizeof(buf), "%d-%b-%Y %H:%M ", &tm);
    std::string str(buf);
    return str;
}


std::string HttpRes::createAutoIndexHtml(std::map<std::string, dir_t> index_of) {
    std::string body = "<html>" "\r\n""<head><title>Index of ";
    body += httpreq.getUri();
    body += "</title></head>" "\r\n""<body>" "\r\n""<h1>Index of ";
    body += httpreq.getUri();
    body += "</h1>" "<hr><pre><a href=\"../\">../</a>" "\r\n";
    for (std::map<std::string, dir_t>::iterator it = index_of.begin(); it != index_of.end(); ++it) {
        body += "<a href=\"";
        body += it->first;
        dir_t info = it->second;
        if ((S_ISDIR(info.d_info.st_mode))) {
            body += '/';
        }
        body += "\">";
        body += it->first; //handle utf ?
        if ((S_ISDIR(info.d_info.st_mode))) {
            body += '/';
        }
        body += "</a> ";

        body += createMtime(info.d_info.st_mtime);
        if ((S_ISDIR(info.d_info.st_mode))) {
            body += "                  -";
        } else {
            std::stringstream ss;
            ss << std::setw(19) << info.d_info.st_size;
            body += ss.str();
        }
        body += "\r\n";
    }
    body += "</pre><hr>";
    body +=  "</body>" "\r\n""</html>" "\r\n";
    return body;
}


std::string HttpRes::joinPathAutoindex() {
	std::string path_root = target.getRoot();
	std::string config_path  = target.getUri();
	std::string file_path = httpreq.getUri().substr(config_path.length());
	std::string alias;
	if ((alias = target.getAlias()) != "") {
		config_path = alias;
	}
	if ((path_root.size() && path_root[path_root.length() - 1] == '/') || path_root.size() == 0) {
		if (config_path.size() >= 1)
			config_path = config_path.substr(1);
	}

	std::cout << "auto index join_path: " << path_root + config_path + file_path << std::endl;
	return path_root + config_path + file_path;
}

int HttpRes::opendirError() {
	if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG) {
		std::cout << "NOT_FOUND" << std::endl;
		status_code = NOT_FOUND;
		return NOT_FOUND;
	} else if (errno == EACCES) {
		std::cout << "FORBIDDEN" << std::endl;
//			status_code = FORBIDDEN;
		status_code = NOT_FOUND;
		return FORBIDDEN;
	}
	std::cout << "INTERNAL_SERVER_ERROR" << std::endl;
	status_code = INTERNAL_SERVER_ERROR;
	return INTERNAL_SERVER_ERROR;
}

int HttpRes::autoindexHandler() {
    std::cout << "================== autoindexHandler ==================" << std::endl;
    std::string req_uri = httpreq.getUri();
    target = getUri2Location(req_uri);
    if (req_uri[req_uri.length() - 1] != '/' || !(target.getIsAutoindex())) {
        return DECLINED;
    }
	std::string method = httpreq.getMethod();
	if (method != "GET" && method != "HEAD") {
        std::cerr << "not allow method in auto_index handler" << std::endl;
		return DECLINED;
	}

	std::vector<std::string> allow_methods = target.get_methods();
	if (find(allow_methods.begin(), allow_methods.end(), method) == allow_methods.end()) {
        status_code = NOT_ALLOWED;
		return status_code;
	}

    if (method == "HEAD") {
        header_only = 1;
    }
    // discard req body

    std::string dir_path = joinPathAutoindex();
    if (dir_path[dir_path.length() - 1] == '/') {
        dir_path = dir_path.substr(0, dir_path.length() - 1);
    }
    dir_t dir_info;
    dir_info.dir = opendir(dir_path.c_str());
	if (dir_info.dir == NULL) {
		return opendirError();
    }
    status_code = HTTP_OK;
    // auto_index only text/html for now
    content_type = "text/html";

    dir_info.valid_info = 0;
    std::map<std::string, dir_t> index_of;
    for (;;) {
        errno = 0;
        dir_info.d_ent = readdir(dir_info.dir);
        if (!dir_info.d_ent) {
            if (errno != 0) {
                std::cerr << "readdir Error" << std::endl;
				status_code = INTERNAL_SERVER_ERROR;
				return status_code;
                // close dir and return error or INTERNAL_SERVER_ERROR
            } else {
                //read directory end
            }
            break;
        }
        std::string file_name(dir_info.d_ent->d_name);
        if (file_name[0] == '.') {
            continue;
        }
        std::string abs_path = joinDirPath(dir_path, file_name);
        if (!dir_info.valid_info) {
            if (stat(abs_path.c_str(), &(dir_info.d_info)) == -1) {
                if (errno == EACCES) {
                    continue;
                } else {
                    status_code = INTERNAL_SERVER_ERROR;
                    return status_code;
                }
            }
            // handle ENOENT or ELOOP -> error or INTERNAL_SERVER_ERROR
        }
        // check link info

        index_of[file_name] = dir_info;
    }
    if (closedir(dir_info.dir) == -1) {
        std::cerr << "closedir Error" << std::endl;
		return status_code;
    }

    if (!header_only) {
        out_buf = createAutoIndexHtml(index_of);
        body_size = out_buf.length();
		content_length_n = body_size;
    }
    sendHeader(); // later ?
    return OK;

}

bool HttpRes::isCgi() {
	Location location = getUri2Location(httpreq.getUri()); //req uri?
	std::cout << "cgi_loc: " << location << std::endl;
    std::vector<std::string> vec = location.getCgiExt();
	if (vec.size() == 0)
		return false;
    std::string path = httpreq.getUri();
	std::vector<std::string>::iterator it = vec.begin();
	for (; it != vec.end(); it++) {
		if (path.find(*it) != std::string::npos)
			return true;
	}
	/*
    if (path.find(vec[0]) != std::string::npos) {
//	if (vec[0] != "") {
		return true;
	}*/
	return false;
}

int HttpRes::checkClientBodySize() {
    if (httpreq.getContentBody() != "") {
	    Location loc = getUri2Location(httpreq.getUri());
        size_t limit_size = loc.getMaxBodySize();
        if (limit_size > 0 && (limit_size < httpreq.getContentLength())) {
            status_code = REQUEST_ENTITY_TOO_LARGE;
            return status_code;
        }
    }
    return OK;
}

void HttpRes::cgiHandler() {
	std::cout << "================== cgi ==================" << std::endl;
	Location location = getUri2Location(httpreq.getUri()); //req uri?
	httpreq.set_meta_variables(location);
	Cgi cgi(httpreq ,location);
	cgi.runCgi();
	int handler_status = 0;
	if (cgi.getStatusCode() > 400) {
		status_code = cgi.getStatusCode();
		return finalizeRes(status_code);
	}
	handler_status = cgi.parseCgiResponse();
  	if (cgi.getResType() == DOCUMENT) {
		std::cout << "===DOCUMENT===" << std::endl;
		status_code = handler_status;
    	cgi.getHeaderFields().erase("status");
    	setCgi(cgi);
    	out_buf = cgi.getCgiBody();
		std::cout << "out_buf: " << out_buf << std::endl;
    	if (cgi.getHeaderFields().count("content-length")) {
			  // ここもutil関数
    	  std::stringstream ss(cgi.getHeaderFields()["content-length"]);
    	  ss >> body_size;
    	} else {
    	  body_size = out_buf.length();
    	}
		content_length_n = body_size;
    	sendHeader(); //tmp here
    	if (httpreq.getMethod() == "HEAD") {
    	  header_only = 1;
    	}
		std::cout << "content_length_n: " << body_size << std::endl;
    	return finalizeRes(status_code);
	} else if (cgi.getResType() == LOCAL_REDIRECT) {
		std::cout << "===LOCAL_REDIRECT===" << std::endl;
		if (httpreq.isRedirectLimit()) {
			status_code = INTERNAL_SERVER_ERROR;
			return finalizeRes(status_code);
		}
		httpreq.setUri(cgi.getHeaderFields()["Location"]);
		httpreq.incrementRedirectCnt();
		return runHandlers();
    } else if (cgi.getResType() == CLIENT_REDIRECT || cgi.getResType() == CLIENT_REDIRECT_WITH_DOC) {
		std::cout << "===OTHER===" << std::endl;
		status_code = MOVED_TEMPORARILY;
		redirect_path = cgi.getHeaderFields()["Location"];
		body = cgi.getCgiBody();
		headerFilter();

		return finalizeRes(status_code);
    } else {
		std::cout << "===NONE===" << std::endl;
		status_code = handler_status;
		body_size = out_buf.length();
    }
    return finalizeRes(status_code);
}

void HttpRes::httpHandler() {
	int handler_status = 0;
	handler_status = returnRedirect();
	if (handler_status != DECLINED) {
		return finalizeRes(handler_status);
	}
    handler_status = staticHandler();
    if (handler_status != DECLINED) {
    	return finalizeRes(handler_status);
    }
    handler_status = autoindexHandler();
    if (handler_status != DECLINED) {
    	return finalizeRes(handler_status);
    }
	handler_status = deleteHandler();
    if (handler_status != DECLINED) {
        return finalizeRes(handler_status);
    }
}

void HttpRes::runHandlers() {
//	int handler_status = 0;
	std::string method = httpreq.getMethod();
    if (method == "HEAD") {
        header_only = 1;
    }
    if (checkClientBodySize() != OK) {
        return finalizeRes(status_code);
    }
	if (isCgi()) {
        return cgiHandler();
	} else {
        return httpHandler();
	}
}
