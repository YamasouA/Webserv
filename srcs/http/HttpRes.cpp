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
	else
		return "";
}

std::string getContentExtension(std::string content_type) {
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

HttpRes::HttpRes()
:status_code(0),
	content_length_n(0),
//    is_posted(0),
	header_only(false),
	keep_alive(0),
    err_status(0),
    is_sended_header(false),
    is_sended_body(false),
	header_size(0),
	body_size(0)
{}

HttpRes::HttpRes(const Client& source)
:status_code(0),
	content_length_n(0),
//    is_posted(0),
	header_only(false),
    err_status(0),
    is_sended_header(false),
    is_sended_body(false),
	header_size(0),
	body_size(0)
{
	this->httpreq = source.getHttpReq();
	this->vServer = source.getVserver();
//	this->fd = source.getFd();
	this->keep_alive = httpreq.getKeepAlive();
}

HttpRes::HttpRes(const HttpRes& src) {
    this->buf = src.buf;
    this->header_size = src.header_size;
    this->out_buf = src.out_buf;
    this->body_size = src.body_size;
    this->is_sended_header = src.getIsSendedHeader();
    this->is_sended_body = src.getIsSendedBody();
	this->keep_alive = src.keep_alive;
	this->header_only = src.header_only;
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
	this->keep_alive = rhs.keep_alive;
	this->header_only = rhs.header_only;
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

//int HttpRes::getStatusCode() const {
//	return status_code;
//}

bool HttpRes::isHeaderOnly() const {
	return header_only;
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

std::string HttpRes::joinPath() {
	std::string path_root = target.getRoot();
	if (path_root == "" || path_root[0] != '/') {
		path_root = "./" + path_root;
	}
	std::string config_path  = target.getUri();
    std::string upload_path = target.getUploadPath();
	std::string file_path = httpreq.getUri();
	if (config_path.length() < file_path.length()) {
		file_path = file_path.substr(config_path.length());
	} else {
		file_path = "";
	}

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
                    return full_path;
                }
            }
        } else {
            return path_root + config_path + file_path + "index.html";
        }
        return path_root + config_path + file_path + *(index_files.begin());
    }
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

//void HttpRes::createContentLength() {
//	std::stringstream ss;
//	std::string code;
//	ss << body.length();
//	header += "Content-Length: ";
//	header += ss.str();
//}

int HttpRes::setContentType() {
	std::string ext;
	std::string type;
	std::string uri = httpreq.getUri();
	std::string::size_type dot_pos = uri.rfind('.');

	if (dot_pos != std::string::npos) {
		ext = uri.substr(dot_pos + 1);
		if (ext.length() == 0) {
			logger.logging("BAD_REQUEST(setContentType)");
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

std::string HttpRes::joinDirPath(const std::string& dir_path, const std::string& elem_name) {
	return dir_path + '/' + elem_name;
}

void HttpRes::divingThroughDir(const std::string& path) {
    dir_t dir_info;
    dir_info.dir = opendir(path.c_str());
    if (dir_info.dir == NULL) {
		logger.logging("INTERNAL_SERVER_ERROR(opendir faile)");
		status_code = INTERNAL_SERVER_ERROR;
		return;
    }
    dir_info.valid_info = 0;
    for (;;) {
        errno = 0;
        dir_info.d_ent = readdir(dir_info.dir);
        if (!dir_info.d_ent) {
            if (errno != 0) {
				logger.logging("INTERNAL_SERVER_ERROR(readdir faile)");
				status_code = INTERNAL_SERVER_ERROR;
            }
            if (closedir(dir_info.dir) == -1) {
				logger.logging("closedir Error");
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
				logger.logging("INTERNAL_SERVER_ERROR(remove faile)");
                status_code = INTERNAL_SERVER_ERROR;
                if (closedir(dir_info.dir) == -1) {
					logger.logging("closedir Error");
                }
                return;
            }
        } else if ((S_ISDIR(dir_info.d_info.st_mode))) {
            divingThroughDir(abs_path);
            if (status_code == INTERNAL_SERVER_ERROR) {
                if (closedir(dir_info.dir) == -1) {
					logger.logging("closedir Error");
                }
                return;
            }
            if (rmdir(abs_path.c_str()) < 0) {
				logger.logging("INTERNAL_SERVER_ERROR(rmdir faile)");
                status_code = INTERNAL_SERVER_ERROR;
                if (closedir(dir_info.dir) == -1) {
					logger.logging("closedir Error");
                }
                return;
            }
        } else {
            if (remove(abs_path.c_str()) < 0) {
				logger.logging("INTERNAL_SERVER_ERROR(remove faile)");
                status_code = INTERNAL_SERVER_ERROR;
                if (closedir(dir_info.dir) == -1) {
					logger.logging("closedir Error");
                }
                return;
            }
      }
    }
}

int HttpRes::deleteError() {
    if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG) {
		logger.logging("NOT_FOUND(deleteError)");
		status_code = NOT_FOUND;
        return NOT_FOUND;
    } else if (errno == EACCES || errno == EPERM) {
		logger.logging("FORBIDDEN(deleteError)");
		status_code = FORBIDDEN;
        return FORBIDDEN;
    } else {
		logger.logging("INTERNAL_SERVER_ERROR(deleteError)");
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
	int content_length = httpreq.getContentLength();
	if (content_length > 0) {
		logger.logging("UNSUPPORTED_MEDIA_TYPE(deleteHandler)");
		status_code = UNSUPPORTED_MEDIA_TYPE;
	}

	struct stat sb;
	bool is_dir;
	std::string method = httpreq.getMethod();
	if (method != "DELETE") {
		return DECLINED;
	}

	std::vector<std::string> allow_methods = target.get_methods();
	if (find(allow_methods.begin(), allow_methods.end(), method) == allow_methods.end()) {
		logger.logging("BAD_REQEUST(deleteHandler)");
		status_code = BAD_REQUEST;
		return status_code;
	}

	std::string file_name = joinPath();
    if (stat(file_name.c_str(), &sb) == -1) {
		logger.logging("NOT_FOUND(deleteHandler)");
		status_code = NOT_FOUND;
		return status_code;
	}
	if (S_ISDIR(sb.st_mode)) {
		std::string uri = httpreq.getUri();
		is_dir = true;
	} else {
		is_dir = false;
	}
	return deletePath(is_dir);
}

void HttpRes::createStatusLine() {
	std::map<int, std::string> status_msg = create_status_msg();
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
                content_type = "";
            }
            status_line = "HTTP/1.1 " + status_code_str + ' ' +  status_msg[status_code];
        } else {
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
	}
	buf += "\r\n";
}

void HttpRes::addContentLengthField() {
    std::stringstream ss;
    ss << content_length_n;
    buf += "Content-Length: " + ss.str();
    buf += "\r\n";
}

void HttpRes::addConnectionField() {
    if (this->keep_alive) {
        buf += "Connection: keep-alive";
    } else {
	    buf += "Connection: close";
    }
	buf += "\r\n";
}

void HttpRes::addLocationField() {
	if (status_code == CREATED && getLocationField() != "") {
		std::string loc_field_value = getLocationField();
		if (loc_field_value[0] == '.') {
			loc_field_value = loc_field_value.substr(1);
		}
        buf += "Location: " + loc_field_value;
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
    } else if (status_code != NO_CONTENT) {
		buf += "Content-Length: 0";
	    buf += "\r\n";
	}
	std::map<std::string, std::string> cgi_headers = cgi.getHeaderFields();
	std::map<std::string, std::string>::iterator it= cgi_headers.begin();
	for (; it != cgi_headers.end(); ++it) {
		if (it->first == "Location")
			continue;
		buf += it->first;
		buf += ": ";
		buf += it->second;
		buf += "\r\n";
	}

	addConnectionField();
	addLocationField();
	buf += "\r\n";
	header_size = buf.size();
}

void HttpRes::sendHeader() {
    // check alredy sent
    if (err_status) {
		origin_status_code = status_code;
        status_code = err_status;
    }
    return headerFilter();
}

int HttpRes::checkAccessToGET(const char *file_name, const std::string& uri) { //or safe method
	if (access(file_name, R_OK) < 0) {
		if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG) {
			if (target.getIsAutoindex() && uri[uri.length() - 1] == '/') {
				return DECLINED;
			} else if (target.getIsAutoindex() == false && uri[uri.length() - 1] == '/') {
				std::string tmp_path = joinPathAutoindex();
				if (access(tmp_path.c_str(), F_OK) == 0) {
					logger.logging("FORBIDDEN(checkAccessToGET)");
					status_code = FORBIDDEN;
					return FORBIDDEN;
				}
			}
			logger.logging("NOT_FOUND(checkAccessToGET)");
			status_code = NOT_FOUND;
			return NOT_FOUND;
		} else if (errno == EACCES){
			logger.logging("FORBIDDEN(checkAccessToGET(EACCESS))");
			status_code = FORBIDDEN;
			return FORBIDDEN;
		}
		logger.logging("INTERNAL_SERVER_ERROR(checkAccessToGET)");
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
		logger.logging("INTERNAL_SERVER_ERROR(stat faile)");
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
		logger.logging("NOT_FOUND(HandleSafeMethod)");
        status_code = NOT_FOUND;
        return NOT_FOUND;
    }
	content_length_n = sb.st_size;
	if (content_length_n == 0) {
		status_code = NO_CONTENT;
		header_only = 1;
	}
	return status_code;
}

int HttpRes::checkAccessToPOST(const char *file_name) {
    if (access(file_name, W_OK) < 0) {
		if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG) {
			logger.logging("NOT_FOUND(checkAccessToPost)");
			status_code = NOT_FOUND;
			return NOT_FOUND;
		} else if (EACCES){
			logger.logging("NOT_FOUND(checkAccessToPost(EACCESS) )");
			status_code = FORBIDDEN;
			return FORBIDDEN;
		}
		logger.logging("INTERNAL_SERVER_ERROR(access faile)");
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
	// 対応していない拡張子かつcontent-typeが存在する場合
	if (ext == "" && httpreq.getHeaderFields()["content-type"] != "") {
		logger.logging("UNSUPPORTED_MEDIA_TYPE");
		status_code = UNSUPPORTED_MEDIA_TYPE;
		return status_code;
	}
    if (file_name[file_name.length() - 1] != '/') {
        file_name = file_name + '/' + ss.str() + ext;
    } else {
        file_name = file_name + ss.str() + ext;
    }
    std::ofstream tmp_ofs(file_name.c_str(), std::ios_base::out);
    if (tmp_ofs.bad()) {
		logger.logging("INTERNAL_SERVER_ERROR(createDestFile)");
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
		if (errno == ENOENT) {
			std::string ext = getContentExtension(httpreq.getHeaderFields()["content-type"]);
			// 対応していない拡張子かつcontent-typeが存在する場合
			if (ext == "" && httpreq.getHeaderFields()["content-type"] != "") {
				logger.logging("UNSUPPORTED_MEDIA_TYPE");
				status_code = UNSUPPORTED_MEDIA_TYPE;
				return status_code;
			}
		    status_code = CREATED;
            setLocationField(file_name);
			std::ofstream tmp_ofs(file_name.c_str(), std::ios_base::out);
			if (tmp_ofs.bad()) {
				logger.logging("INTERNAL_SERVER_ERROR(stat faile ENOENT)");
				status_code = INTERNAL_SERVER_ERROR;
				return INTERNAL_SERVER_ERROR;
			}
			tmp_ofs.close();
		} else if (errno == ENOTDIR || errno == ENAMETOOLONG) {
			logger.logging("NOT_FOUND(handlePost)");
            status_code = NOT_FOUND;
            return status_code;
        } else {
			logger.logging("INTERNAL_SERVER_ERROR(stat faile)");
            status_code = INTERNAL_SERVER_ERROR;
            return status_code;
        }
    } else {
        status_code = HTTP_OK;
        std::string ext = getContentExtension(httpreq.getHeaderFields()["content-type"]);
			// 対応していない拡張子かつcontent-typeが存在する場合
		if (ext == "" && httpreq.getHeaderFields()["content-type"] != "") {
			logger.logging("UNSUPPORTED_MEDIA_TYPE");
			status_code = UNSUPPORTED_MEDIA_TYPE;
			return status_code;
		}

		if (S_ISDIR(sb.st_mode)) {
			if (createDestFile(file_name) != OK) {
				return status_code;
			}
			status_code = CREATED;
			setLocationField(file_name);
		}
		if (!S_ISREG(sb.st_mode) && status_code != CREATED) {
			logger.logging("INTERNAL_SERVER_ERROR(stat faile)");
			return INTERNAL_SERVER_ERROR;
		} else {
			content_length_n = sb.st_size;
		}
    }
	if (checkAccessToPOST(file_name.c_str()) != OK) {
		return status_code;
	}
	std::ofstream ofs(file_name.c_str(), std::ios::app);
	if (!ofs) {
		logger.logging("INTERNAL_SERVER_ERROR(POST open faile)");
		status_code = INTERNAL_SERVER_ERROR;
		return INTERNAL_SERVER_ERROR;
	}
	std::string body = httpreq.getContentBody();
	if (body.length() == 0 && content_length_n == 0 && status_code != CREATED) {
		status_code = NO_CONTENT;
		header_only = 1;
	}
	ofs << body;
	ofs.close();
	return OK;
}

int HttpRes::handleResBody(const std::string& file_name) {
    if (!header_only) {
		std::ifstream ifs(file_name.c_str(), std::ios::binary);
		if (!ifs) {
			logger.logging("INTERNAL_SERVER_ERROR(handleResBody)");
			status_code = INTERNAL_SERVER_ERROR;
			return status_code;
		}
        std::ostringstream oss;
        oss << ifs.rdbuf();
        out_buf = oss.str();
		content_length_n = out_buf.length();
		body_size = out_buf.length();
		return OK;
    }
	return OK;

}

int HttpRes::staticHandler() {
	std::string uri = httpreq.getUri();
	target = getUri2Location(uri);
    if (target.getUri() == "") {
		logger.logging("NOT_FOUND(staticHandler)");
        status_code = NOT_FOUND;
        return status_code;
    }
	std::string method = httpreq.getMethod();
	if (method != "GET" && method != "HEAD" && method != "POST") {
		return DECLINED;
	}
	std::vector<std::string> allow_methods = target.get_methods();
	if (find(allow_methods.begin(), allow_methods.end(), method) == allow_methods.end()) {
		logger.logging("NOT_ALLOWED(staticHandler)");
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
		} else if (handler_status == OK) {
			return OK;
		}
    } else if (method == "POST") {
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

    return err_page_buf;
}

int HttpRes::sendErrorPage() {
    std::string path = target.getErrorPage(status_code);
    if (path[0] == '/') {
		httpreq.setUri(path);
		if (buf.length()) {
			buf.erase();
		}
		if (out_buf.length()) {
			out_buf.erase();
		}
		staticHandler();
		if (200 <= origin_status_code && origin_status_code < 300) {
//		if (out_buf.length() == 0) {
			return OK;
//			out_buf = createErrPage();
		}
		return DECLINED;
	}
	return DECLINED;
}

int HttpRes::redirectHandle() {
    err_status = status_code;
    switch (status_code) {
        case BAD_REQUEST:
        case REQUEST_ENTITY_TOO_LARGE:
		case REQUEST_TIME_OUT:
		case LENGTH_REQUIRED:
        case REQUEST_URI_TOO_LARGE:
        case INTERNAL_SERVER_ERROR:
        case HTTP_NOT_IMPLEMENTED:
            keep_alive = 0;
    }
    content_type.erase();

    if (target.getErrorPage(status_code) != "") {
        int flag = sendErrorPage();
		if (flag == OK) {
			return OK;
		}
    }
//     discard request body
    if (out_buf.length()) {
        out_buf.erase();
        content_length_n = 0;
    }

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
    sendHeader();
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
	std::string uri = httpreq.getUri();
	Location loc = getUri2Location(uri);
	std::string ret = loc.getReturn();
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
		} else {
			return DECLINED; //ERROR;
		}
	} else {
		if (status_code > 999) {
			return DECLINED; //ERROR;
		}
		if (elms.size() == 1) {
			return DECLINED; //OK;
		}
		path = elms[1];
	}
	redirect_path = path;
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

	return path_root + config_path + file_path;
}

int HttpRes::opendirError() {
	if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG) {
		logger.logging("NOT_FOUND(opendirError)");
		status_code = NOT_FOUND;
		return NOT_FOUND;
	} else if (errno == EACCES) {
		logger.logging("FORBIDDEN(opendirError)");
		status_code = FORBIDDEN;
		return FORBIDDEN;
	}
	logger.logging("INTERNAL_SERVER_ERROR(opendirError)");
	status_code = INTERNAL_SERVER_ERROR;
	return INTERNAL_SERVER_ERROR;
}

int HttpRes::autoindexHandler() {
    std::string req_uri = httpreq.getUri();
    target = getUri2Location(req_uri);
    if (req_uri[req_uri.length() - 1] != '/' || !(target.getIsAutoindex())) {
        return DECLINED;
    }
	std::string method = httpreq.getMethod();
	if (method != "GET" && method != "HEAD") {
		return DECLINED;
	}

	std::vector<std::string> allow_methods = target.get_methods();
	if (find(allow_methods.begin(), allow_methods.end(), method) == allow_methods.end()) {
		logger.logging("NOT_ALLOWED(autoindexHandler)");
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
				logger.logging("INTERNAL_SERVER_ERROR(readdir Error)");
				status_code = INTERNAL_SERVER_ERROR;
				return status_code;
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
					logger.logging("INTERNAL_SERVER_ERROR(stat Error (not EACCES) )");
                    status_code = INTERNAL_SERVER_ERROR;
                    return status_code;
                }
            }
        }

        index_of[file_name] = dir_info;
    }
    if (closedir(dir_info.dir) == -1) {
		logger.logging("closedir Errror");
		return status_code;
    }

    if (!header_only) {
        out_buf = createAutoIndexHtml(index_of);
        body_size = out_buf.length();
		content_length_n = body_size;
    }
    sendHeader();
    return OK;

}

bool HttpRes::isCgi() {
	Location location = getUri2Location(httpreq.getUri());
    std::vector<std::string> vec = location.getCgiExt();
	if (vec.size() == 0)
		return false;
//    std::string path = httpreq.getUri();
	httpreq.setMetaVariables(location);
    std::string path = httpreq.get_meta_variables()["SCRIPT_NAME"];

	std::vector<std::string>::iterator it = vec.begin();
	for (; it != vec.end(); it++) {
		if (path.find(*it) != std::string::npos)
			return true;
	}
	return false;
}

int HttpRes::checkClientBodySize() {
    if (httpreq.getContentBody() != "") {
	    Location loc = getUri2Location(httpreq.getUri());
        size_t limit_size = loc.getMaxBodySize();
        if (limit_size > 0 && (limit_size < httpreq.getContentLength())) {
			logger.logging("REQUEST_ENTITY_TOO_LARGE");
            status_code = REQUEST_ENTITY_TOO_LARGE;
            return status_code;
        }
    }
    return OK;
}

void HttpRes::cgiHandler() {
	Location location = getUri2Location(httpreq.getUri()); //req uri?
	target = location;
//	httpreq.setMetaVariables(location);
	Cgi cgi(httpreq ,location);
	cgi.runCgi();
	int handler_status = 0;
	if (cgi.getStatusCode() > 400) {
		status_code = cgi.getStatusCode();
		return finalizeRes(status_code);
	}
	handler_status = cgi.parseCgiResponse();
  	if (cgi.getResType() == DOCUMENT) {
		status_code = handler_status;
    	cgi.getHeaderFields().erase("status");
    	setCgi(cgi);
    	out_buf = cgi.getCgiBody();
    	if (cgi.getHeaderFields().count("content-length")) {
			  // ここもutil関数
		  std::string content_length_str = cgi.getHeaderFields()["content-length"];
		  if (content_length_str.find_first_not_of("0123456789") != std::string::npos) {
			status_code = HTTP_BAD_GATEWAY;
			return finalizeRes(status_code);
		  }

    	  std::stringstream ss(content_length_str);
    	  ss >> body_size;
		  if (ss.bad()) {
			  logger.logging("INTERNAL_SERVER_ERROR(stream is broken)");
			  status_code = INTERNAL_SERVER_ERROR;
			  return finalizeRes(status_code);
		  }
		  if (ss.fail() && body_size == std::numeric_limits<size_t>::max()) {
			status_code = HTTP_BAD_GATEWAY;
			return finalizeRes(status_code);
		  }
    	} else {
    	  body_size = out_buf.length();
    	}
		content_length_n = body_size;
    	sendHeader(); //tmp here
    	if (httpreq.getMethod() == "HEAD") {
    	  header_only = 1;
    	}
    	return finalizeRes(OK);
	} else if (cgi.getResType() == LOCAL_REDIRECT) {
		if (httpreq.isRedirectLimit()) {
			logger.logging("INTERNAL_SERVER_ERROR(Cgi Redirect limit)");
			status_code = INTERNAL_SERVER_ERROR;
			return finalizeRes(status_code);
		}
		httpreq.setUri(cgi.getHeaderFields()["Location"]);
		httpreq.incrementRedirectCnt();
		return runHandlers();
    } else if (cgi.getResType() == CLIENT_REDIRECT || cgi.getResType() == CLIENT_REDIRECT_WITH_DOC) {
		status_code = MOVED_TEMPORARILY;
		redirect_path = cgi.getHeaderFields()["Location"];
		body = cgi.getCgiBody(); //body? out_buf?
		headerFilter();

		return finalizeRes(OK);
    } else {
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
