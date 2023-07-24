#include "HttpRes.hpp"
#include "../Client.hpp"

const std::string HttpRes::kServerName = "WebServe";
const std::string HttpRes::default_type = "text/html";
//static const std::map<std::string, std::string> types = {{"html", "text/html"},{"json", "application/json"}};

std::string getContentType(std::string type) {
	if (type == "html")
		return "text/html";
	else if (type == "json")
		return "application/json";

	return "";
}

HttpRes::HttpRes() {

}

HttpRes::HttpRes(const Client& source, Kqueue &kq)
:content_length_n(0),
is_posted(0),
    err_status(0)
{
	//this->httpreq = source.get_parsedReq();
	this->httpreq = source.get_httpReq();
	this->vServer = source.get_vServer();
    this->connection = &kq;
	this->fd = source.get_fd();
}

HttpRes::HttpRes(const HttpRes& src) {
    this->buf = src.buf;
    this->header_size = src.header_size;
    this->out_buf = src.out_buf;
    this->body_size = src.body_size;
}

HttpRes::~HttpRes() {
	//close(fd);
}


Location HttpRes::get_uri2location(std::string uri) const
{
	//std::cout << "===== get_uri2location() =====" << std::endl;
//    std::vector<Location> locs = vServer.get_locations();
//    std::map<std::string, Location> uri2loc;
//    for (std::vector<Location>::iterator it = locs.begin(); it != locs.end(); ++it) {
//        uri2loc[it->get_uri()] = *it;
//    }
	std::map<std::string, Location> uri2location = vServer.get_uri2location();
	std::map<std::string, Location>::const_iterator loc = uri2location.find(uri);
	//std::cout << "request uri: " << uri << std::endl;
	//std::cout << "size: " << uri2location.size() << std::endl;
	/*
	for (std::map<std::string, Location>::iterator it = uri2location.begin();
		it != uri2location.end(); it++) {
		//std::cout << "location uri: " << it->second.get_uri() << std::endl;
	}*/
	if (loc != uri2location.end()) {
        //std::cout << "match all" << std::endl;
		return loc->second;
	}
	/*
	if (uri2location.count(uri) == 1)
		return uri2location[uri];
	*/
	std::string path = uri;
	while (1) {
		std::string::size_type i = path.rfind('/');
//		std::cout << "i: " << i << std::endl;
//		std::cout << "a" << std::endl;
		if (i == std::string::npos) {
			//std::cout << "a" << std::endl;
            //std::cout << "okend" << std::endl;
			break;
		}
//		std::cout << "b" << std::endl;
		//std::cout << "path(bef): " << path << std::endl;
		if (i == 0) {
			path = path.substr(0, 1);
        } else {
		    path = path.substr(0, i);
        }
		//std::cout << "path: " << path << std::endl;
//        }
        //std::cout << "substr path: " << path << std::endl;
		loc = uri2location.find(path);
//		std::map<std::string, Location>::const_iterator loc = uri2location.find(path);
		if (loc != uri2location.end()) {
            //std::cout << "ok1" << std::endl;
			return loc->second;
		}

//		if (uri2location.count(path) == 1)
//			return uri2location[path];

	}

	//std::cout << "break" << std::endl;
	// 何もマッチしなかった場合の挙動イズなに？
	return loc->second;
}



Location HttpRes::longestMatchLocation(std::string request_path, std::vector<Location> locations) {
	Location location;
	size_t max_len = 0;
	for (std::vector<Location>::iterator it = locations.begin(); it != locations.end(); it++) {
        std::string location_path = it->get_uri();
		// locationの方が長い場合はマッチしない
		//std::cout << "request_path: " << request_path << std::endl;
		//std::cout << "location_path: " << location_path << std::endl;
		if (request_path.find(location_path) == 0) {
			// 終端が'/'でないまたは、最後のスペルまでマッチしている以外は
			// 中途半端にマッチしていて間違っている
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

std::string HttpRes::join_path() {
//    std::cout << "===== join_path =====" << std::endl;
//	std::cout << "location: " <<  location << std::endl;
	std::string path_root = target.get_root();
	std::string config_path  = target.get_uri();
	std::string file_path = httpreq.getUri().substr(config_path.length());
	/*
						|            request uri       |
		/User/root/path/ /config/location/ /file_path.html
		path_root         config_path     file_path
	*/
//	std::cout << "root: " << path_root << std::endl;
//	std::cout << "config: " << config_path << std::endl;
//	std::cout << "file: " << file_path << std::endl;
	if (!file_path.length() && config_path[config_path.length() - 1] == '/' && (target.get_index().length() != 0 || target.get_is_autoindex())) { // actually not autoindex, Completely different directive index directive
//	if (!file_path.length() && config_path[config_path.length() - 1] == '/' && target.get_index_file() {
	    if (config_path == "/") {
		    config_path = "";
        }
	    file_path = "/index.html"; // from index directive
	}
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
//		if (file_path.size() >= 1) { //
//			file_path = file_path.substr(1);
//        }
	}
	//std::cout << "path: " << path_root + config_path + file_path << std::endl;
	std::cout << "join_path: " << path_root + config_path + file_path << std::endl;
//    std::cout << "===== End join_path =====" << std::endl;
	return path_root + config_path + file_path;
}

void HttpRes::set_body(std::string strs)
{
    this->body = strs;
}

void HttpRes::set_cgi(Cgi cgi) {
    this->cgi = cgi;
}

Cgi HttpRes::get_cgi() const {
    return cgi;
}

void HttpRes::read_file() {
	std::string buf[1024];

	// locationから正しいファイルパスを作成する
	std::string file_name = join_path();
//    std::cout << file_name << std::endl;
	std::ifstream ifs(file_name.c_str());
	if (!ifs) {
		// file not foundかそれ以外かをどう見分けるか
		status_code = 404;
		//std::cout << "ko" << std::endl;
	}
	std::ostringstream oss;
	oss << ifs.rdbuf();
	status_code = 200;
	const std::string strs = oss.str();
	set_body(strs);
	//std::cout << body << std::endl;
}

void HttpRes::write_file() {
	//std::cout << "Write" << std::endl;
	std::string file_name = join_path();
	//std::cout << "file_name: " << file_name << std::endl;
	std::ofstream ofs(file_name);
	if (!ofs.is_open()) {
		//std::cout << "Error(ofstream)" << std::endl;
	}
	ofs << httpreq.getContentBody();
}

void HttpRes::delete_file() {
	//std::cout << "Delete" << std::endl;
	std::string file_name = join_path();
	//std::cout << "file_name: " << file_name << std::endl;
	// cのremoveなら使っても大丈夫??
	// c++だと新しくてダメっぽい
	if (remove(file_name.c_str())) {

	}
}
/*
std::string HttpRes::get_path(std::string str) {
}*/

std::string HttpRes::getStatusString() {
	switch (status_code) {
		case 200:
			return "OK\n";
		case 404:
			return "Not Found\n";
	}
	return "Error(statusString)\n";
}

void HttpRes::createControlData() {
	//body += httpreq.getVersion();
	header += "HTTP1.1 ";
	std::stringstream ss;
	std::string code;
	//std::cout << "status_code: " << status_code << std::endl;
	ss << status_code;
	//std::cout << "code: " << code << std::endl;
	header += ss.str();
	header += " ";
	header += getStatusString();
}

//void HttpRes::createDate()
std::string HttpRes::createDate(time_t now, std::string fieldName)
{
    std::string str;
    char buf[1000];
    //time_t now = time(0);
    struct tm tm = *gmtime(&now);
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S ", &tm);
    //body += "Date: ";
	str += fieldName + ": ";
//	header += fieldName + ": ";
    std::string date(buf);
    str += date + "GMT\n";
//    header += date + "GMT\n";
    return str;
}

void HttpRes::createContentLength() {
	std::stringstream ss;
	std::string code;
	ss << body.length();
	//std::cout << "code: " << code << std::endl;
	header += "Content-Length: ";
	header += ss.str();
}

void HttpRes::set_content_type() {
	std::string ext;
	std::string type;
	std::string uri = httpreq.getUri();
	std::string::size_type dot_pos = uri.find('.');

	// .のみのケースに対応できるか？
	if (dot_pos != std::string::npos) {
		ext = uri.substr(dot_pos + 1);
		if (ext.length() == 0) {
			std::cerr << "Error dot" << std::endl;
            abort();
		}
	}
	for (size_t i = 0; i < ext.length(); i++) {
		if (ext[i] >= 'A' && ext[i] <= 'Z') {
			type += std::tolower(ext[i]);
		} else {
			type += ext[i];
		}
	}

	// content-typeが受けられるか
	/*
	if (types.count(type) != 0) {
		content_type = types[type];
	}*/
	content_type = getContentType(type);

	// マッチしなかったらデフォルトの値をセットする
    if (content_type.length() == 0) {
	    content_type = default_type;
    }
}

void HttpRes::ev_queue_insert() {
	//connection.set_event(fd, EVFILT_WRITE);
	connection->set_event(fd, EVFILT_WRITE);
    std::cout << "==================send write event==================" << std::endl;
}

void HttpRes::post_event() {
    if (!is_posted) {
        is_posted = 1;
        ev_queue_insert();
    }
//    std::cout << "posted event" << std::endl;
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

int HttpRes::dav_depth() {
	// アクセスできるディレクトリの深さを返す
	int depth = target.get_depth();
	return depth;
}



std::string HttpRes::join_dir_path(const std::string& dir_path, const std::string& elem_name) {
	return dir_path + '/' + elem_name;
}

void HttpRes::diving_through_dir(const std::string& path) {
    //class is better?
    dir_t dir_info;
    dir_info.dir = opendir(path.c_str());
    if (dir_info.dir == NULL) {
        std::cerr << "opendir Error" << std::endl;
    }
    dir_info.valid_info = 0;
    for (;;) {
        errno = 0;
        dir_info.d_ent = readdir(dir_info.dir);
        if (!dir_info.d_ent) {
            if (errno != 0) {
              std::cerr << "readdir Error" << std::endl;
            } else {
            // read directory end
            }
            if (closedir(dir_info.dir) == -1) {
                std::cerr << "closedir Error" << std::endl;
            }
            return;
              // done handle
        }
        std::string file_name(dir_info.d_ent->d_name);
        if (file_name.length() == 1 && file_name[0] == '.') {
            continue;
        }
        if (file_name.length() == 2 && file_name[0] == '.' && file_name[1] == '.') {
            continue;
        }
        std::string abs_path = join_dir_path(path, file_name);
        //std::cout << "abs_path: " << abs_path << std::endl;

        if (!dir_info.valid_info) {
//            dir_info.type = 0;
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
            diving_through_dir(abs_path);
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

int HttpRes::delete_error() {
    if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG) {
        return NOT_FOUND;
    } else if (errno == EACCES || errno == EPERM) {
        return FORBIDDEN;
    } else {
        return INTERNAL_SERVER_ERROR;
    }
}

int HttpRes::dav_delete_path(bool is_dir) {
    if (is_dir) {
        std::string dir_path = join_path();
        //std::cout << "dir_path: " << dir_path << std::endl;
        diving_through_dir(dir_path);
        if (status_code == INTERNAL_SERVER_ERROR) {
            return status_code;
        }
        if (rmdir(dir_path.c_str()) >= 0) {
			//std::cout << "delete error" << std::endl;
            status_code = NO_CONTENT;
            return status_code;
//			status_code = INTERNAL_SERVER_ERROR;
//            return status_code;
        }

		// 本当ならディレクトリ配下を確認して問題なければディレクトリを消すべき
		// めんどいからディレクトリの削除を行わせない？
//		status_code = -1;
	} else {
		std::string file_name = join_path();
		//file_name = "hogehoge.txt";
		//std::cout << "delete!!" << std::endl;
		if (remove(file_name.c_str()) >= 0) {
			//std::cout << "delete error" << std::endl;
		    status_code = NO_CONTENT;
			return status_code;
		}
//        status
	}
    return delete_error();
//    return status_code;
}


//void HttpRes::dav_delete_handler() {
int HttpRes::dav_delete_handler() {
	std::cout << "====================dav delete handler====================" << std::endl;
	int content_length = httpreq.getContentLength();
	if (content_length > 0) {
		status_code = UNSUPPORTED_MEDIA_TYPE;
	}

	struct stat sb;
	bool is_dir;
	int depth;
	//std::cout << "a" << std::endl;
	std::string method = httpreq.getMethod();
	if (method != "DELETE") {
		return DECLINED;
	}

	std::vector<std::string> allow_methods = target.get_methods();
	//std::cout << "allow_methods: " << std::endl;
	if (find(allow_methods.begin(), allow_methods.end(), method) == allow_methods.end()) {
		std::cout << "not allow (conf)" << std::endl;
		status_code = BAD_REQUEST;
		return status_code;
	}

	std::string file_name = join_path();
//	file_name = "abc";
    if (stat(file_name.c_str(), &sb) == -1) {
		std::cout << "Error(stat)" << std::endl;
		status_code = INTERNAL_SERVER_ERROR;
		return status_code;
	}
	if (S_ISDIR(sb.st_mode)) {
		std::string uri = httpreq.getUri();
        //std::cout << "dir uri: " << uri << std::endl;
//		if (uri[uri.length() - 1] != '/') {
//			status_code = BAD_REQUEST;
//			return;
//		}
        //std::cout << "ok1" << std::endl;
		depth = dav_depth();
		if (depth != -1) {
			status_code = BAD_REQUEST;
			return status_code;
		}
		is_dir = true;
        //std::cout << "ok" << std::endl;
	} else {
		//std::cout << "delete files" << std::endl;
		depth = dav_depth();
		//std::cout << "depth: " << depth << std::endl;
		if (depth != 0 && depth != -1) {
			status_code = BAD_REQUEST;
			return status_code;
		}
		is_dir = false;
	}
	return dav_delete_path(is_dir);
}

void HttpRes::header_filter() {
	// ステータスがOKでないならlast_modifiedは消す
	std::map<int, std::string> status_msg = create_status_msg();
	if (last_modified_time != -1) {
		if (status_code != HTTP_OK) {
			last_modified_time = -1;
		}
	}

	// status_lineの作成
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
                header_only = 1;
                content_type = "";
                last_modified = NULL;
            }
            status_line = "HTTP/1.1 " + status_code_str + ' ' +  status_msg[status_code];
        } else {
            //
			status_line = "";
		}
	}

	// めっちゃlenを操作してる箇所はいらなさそうだから飛ばす

	buf += status_line;
	buf += "\r\n";
	// ServerNameも設定できるぽいけど挙動よくわからん
	buf += "Server: " + kServerName;

	buf += "\r\n";
	// Cacheとかは考慮しないのでdateの処理は飛ばす
    time_t now = std::time(NULL);
    buf += createDate(now, "Date");
	if (content_type != "") {
		buf += "Content-Type: " + content_type;

		if (charset != "") {
			buf += "; charset=" + charset;

			// content_type に charsetを加える
		}
		buf += "\r\n";
	} if (content_length_n > 0) { // Is this process ok?
        std::stringstream ss;
        ss << content_length_n;
        buf += "Content-Length: " + ss.str();
	    buf += "\r\n";
    }
	// content_length_n と content_lengthの関係がよくわからん
	if (last_modified_time != -1) {
		//buf += "Last-Modified: " + http_time();
//		buf += "\r\n";
	}

	// Locationの処理いろいろやってそう

	// chunkedは無視

	std::map<std::string, std::string> cgi_headers = cgi.getHeaderFields();
	std::map<std::string, std::string>::iterator it= cgi_headers.begin();
	for (; it != cgi_headers.end(); ++it) {
//        std::cout << "cgi i: " << i << std::endl;
		if (it->first == "Location")
			continue;
        std::cout << it->first << ": " << it->second << std::endl;
		buf += it->first;
		buf += ": ";
		buf += it->second;
		buf += "\r\n";
	}

    // keep-alive系は問答無用でcloseする？
    // keep-alive looks better managed with flags.
//    std::map<std::string, std::string> header_fields = httpreq.getHeaderFields();
//    if (header_fields["connection"] == "keep-alive") {
    if (httpreq.getKeepAlive()) {
        buf += "Connection: keep-alive";
    } else {
	    buf += "Connection: close";
    }
	//buf += "Connection: close";
	buf += "\r\n";
	if (status_code >= 300 && status_code < 400 && redirect_path.length()> 0) {
		buf += "Location: " + redirect_path;
		buf += "\r\n";
	}
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
    post_event();
	//write_filter();
}

void HttpRes::sendHeader() {
    // check alredy sent
    if (err_status) {
        status_code = err_status;
    }
    return header_filter();
}

int HttpRes::static_handler() {
	std::cout << "================== static_handler ==================" << std::endl;
	std::string uri = httpreq.getUri();
	target = get_uri2location(uri);
    //std::cout << "macth loc: " << target << std::endl;
	std::string method = httpreq.getMethod();
	if (method != "GET" && method != "HEAD" && method != "POST") {
        std::cerr << "not allow method in static handler" << std::endl;
		// なんてエラー返そう？
		return DECLINED;
	}
	std::vector<std::string> allow_methods = target.get_methods();
	//std::cout << "allow_methods: " << std::endl;
	if (find(allow_methods.begin(), allow_methods.end(), method) == allow_methods.end()) {
		//std::cout << "not allow (conf)" << std::endl;
		status_code = BAD_REQUEST;
		return DECLINED;
	}

//	if (uri[uri.length() - 1] == '/' && !target.get_is_autoindex()) {
	if (uri[uri.length() - 1] == '/' && !target.get_index().length() && !target.get_is_autoindex()) {
        //move next handler
		// なんて返す？ (declined)
		return DECLINED;
	}

	//rc = ngx_http_discard_body(r);

	//map_uri_to_path();

	int _fd = -1;
	std::string file_name = join_path();
	//file_name = "index.html";
//    std::cout << "file_name: " << file_name << std::endl;
//	std::cout << "method: " << method << std::endl;
    struct stat sb;
    status_code = 200;
    if (method == "GET") {
        _fd = open(file_name.c_str(), O_RDONLY);
//		std::cout << "RES GET fd: " << _fd << std::endl;
        if (_fd == -1) {
            std::cerr << "open Error" << std::endl;
            if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG) {
                if (target.get_is_autoindex() && uri[uri.length() - 1] == '/') {
//                } else if (!target.get_index().length() && target.get_is_autoindex()) {
                    return DECLINED;
                } else if (target.get_index().length() > 0 && uri[uri.length() - 1] == '/') {
                    std::cout << "FORBIDDEN" << std::endl;
                    status_code = FORBIDDEN;
                    return FORBIDDEN;
                }
                std::cout << "NOT FOUND" << std::endl;
                status_code = NOT_FOUND;
                return NOT_FOUND;
            } else if (EACCES){
                std::cout << "FORBIDDEN" << std::endl;
                status_code = FORBIDDEN;
                return FORBIDDEN;
            }
            // 次のハンドラーに処理を託す
            status_code = INTERNAL_SERVER_ERROR;
            return INTERNAL_SERVER_ERROR;
        }
        // stat がエラーだったとき
        if (stat(file_name.c_str(), &sb) == -1) {
            std::cout << "GET Error(stat)" << std::endl;
            abort();
        }
        // ディレクトリだった時
        if (S_ISDIR(sb.st_mode)) {
			close(_fd);
            return DECLINED;
        }
    //    if (!S_ISREG(sb.st_mode) && method == "POST") {
    //        //status 405
    //        std::cerr << "NOT ALLOW METHOD" << std::endl;
    //        return NOT_ALLOWED;
    //    }
        // 通常ファイルではない
        if (!S_ISREG(sb.st_mode)) {
			close(_fd);
            // なんのエラー?
            std::cerr << "NOT FOUND(404)" << std::endl;
            //status 404
            status_code = NOT_FOUND;
            return NOT_FOUND;
        }
	    content_length_n = sb.st_size;
	    last_modified_time = sb.st_mtime;
    } else if (method == "POST") {
        if (stat(file_name.c_str(), &sb) == -1) {
			// ファイルが存在しない
			if (errno != ENOENT) {
				close(_fd);
                std::cerr << "stat Error" << std::endl;
                return INTERNAL_SERVER_ERROR;
				// throw error
			}
			status_code = 201;
        }
        // ディレクトリだった時
        if (S_ISDIR(sb.st_mode)) {
			close(_fd);
            return DECLINED;
        }
    //    if (!S_ISREG(sb.st_mode) && method == "POST") {
    //        //status 405
    //        std::cerr << "NOT ALLOW METHOD" << std::endl;
    //        return NOT_ALLOWED;
    //    }
        // 通常ファイルではない
        content_length_n = sb.st_size;
	    last_modified_time = sb.st_mtime;
        if (!S_ISREG(sb.st_mode) && status_code != 201) {
			std::cerr << "stat Error" << std::endl;
			close(_fd);
        	return INTERNAL_SERVER_ERROR;
			/*
            fd = open(file_name.cstr(), O_CREATE | O_WRONLY); //open necessary???
            if (fd == -1) {
                std::cerr << "open error" << std::endl;
                return INTERNAL_SERVER_ERROR;
            }
            status_code = 201;
			*/

        } else {
			// 通常ファイル
			//file_name = "sinki.html";
			_fd = open(file_name.c_str(), O_CREAT | O_WRONLY | O_APPEND, 00644);
			if (_fd == -1) {
                std::cerr << "POST open Error" << std::endl;
                return INTERNAL_SERVER_ERROR;
			}
			std::string body = httpreq.getContentBody().c_str();
//			std::cout << "POST body: \n" << body << std::endl;
			write(_fd, body.c_str(), body.size());
		}
    }
    close(_fd);
    //discoard request body here ?
	set_content_type();
    //set_etag(); //necessary?
//    std::cout << "just before sendHeader" << std::endl;
    sendHeader();

//    init_res_body();

    std::ifstream ifs(file_name.c_str(), std::ios::binary);
    if (!ifs) {
        std::cerr << "ifstream ko" << std::endl;
        abort();
    }
    std::ostringstream oss;
    oss << ifs.rdbuf();
    out_buf = oss.str();
	//buf += out_buf;
    std::cout << "response body: " << std::endl;
    std::cout << out_buf << std::endl;
    body_size = content_length_n;
//	std::cout << "===== End static_handler =====" << std::endl;
    return OK;
//    output_filter();
}

std::string HttpRes::create_err_page() {
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

// handle return derective ??? or handle only error_page directive ?
int HttpRes::send_error_page() {
    std::string path = target.get_error_page(status_code);
//    std::cout << "error_page_path: " << path << std::endl;
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
        //return internal_redirect
    }
    //if path[0] == '@' non-supported
    //  nameed_location
    //
    //discard request body
    //
    // ovar_write ??
    //if !MOVED_PERMANENTLY &&
    //   !MOVED_TEMPORARILY &&
    //   !SEE_OTHER &&
    //   !TEMPORARY_REDIRECT &&
    //   !PERMANENT_REDIRECT
    //      err_status = MOVED_TEMPORARILY;
    //
    //
    //clear location
    //location = location info
    //location_field = location
	return 0;
}

int HttpRes::redirect_handler() {
    std::cout << "================== redirect_handler ==================" << std::endl;
    err_status = status_code;
    switch (status_code) {
        case BAD_REQUEST:
//        case REQUEST_ENTITY_TOO_LARGE:
//        case REQUEST_URI_TOO_LARGE:
//        case HTTP_TO_HTTPS:
//        case HTTPS_CERT_ERROR:
//        case HTTPS_NO_CERT:
        case INTERNAL_SERVER_ERROR:
        case HTTP_NOT_IMPLEMENTED:
            keep_alive = 0;
    }
    content_type.erase();

//    if (have_error_pages == 1) {// have err_page directive
         //err_pages = from conf
//         for (size_t i = 0; i < err_pages_num; ++i) {
//    std::cout << "=========check error_page============: " << target.get_error_page(status_code) << std::endl;
    if (target.get_error_page(status_code) != "") {
        return send_error_page();
    }
//            }
//     }
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
    // create new tmp file ?
    // or map<status_code, err_page_content> ?
    // if We create a new file, how do We handle mtime?
//    std::string err_page_buf = error_pages[status_code];
    std::string err_page_buf = std::string();
    if (status_code >= 300) {
        err_page_buf = create_err_page();
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
//    std::cout << "====ok====" << std::endl;
    sendHeader();
//    if err || only_header
//        return
//    if content_length == 0
        // something
    out_buf = err_page_buf;
    body_size = content_length_n;
    return OK;
}

void HttpRes::finalize_res(int handler_status)
{
	std::cout << "================== finalize_res ==================" << std::endl;
    if (handler_status == DECLINED || handler_status == OK) {
        return;
    }
    if ((200 <= status_code && status_code < 207) && status_code != 204) {// || err_status > 0) {//except 201, 204 ? //or DONE, OK
        // handle connection
        return;
    }
    if (status_code >= 300 || status_code == 204) {//and 201, 204 ?
        // handle around timeer
        //
//        std::cout << "status code over 300 case" << std::endl;
        redirect_handler(); //recurcive finalize_res is better?
        return;
    }
}

int HttpRes::return_redirect() {
	std::cout << "================== return redirect ==================" << std::endl;
	std::string uri = httpreq.getUri();
	Location loc = get_uri2location(uri);
	std::string ret = loc.get_return();
//	std::cout << "return: " << ret << std::endl;
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
//	std::cout << "status: " << status_code << std::endl;
	std::string path;
	// 1引数目が数値じゃない場合(URIが入ってる)
	if (ss.fail()) {
		// URLのスキームが正常か？
		//if (uri.startswith("http://") || uri.startswith("https://")) {
		path = elms[0];
//		std::cout << "path(bef): " << path << std::endl;
		if (!path.compare(0, 7, "http://") || !path.compare(0, 8, "https://")) {
			status_code = MOVED_TEMPORARILY;
//            return status_code;
			//path = elms[0];
		} else {
			std::cout << "scheme Error" << std::endl;
			return DECLINED; //ERROR;
		}
	} else {
		// statusの値が正常値じゃない場合(999)
		if (status_code > 999) {
			std::cout << "status_code Error" << std::endl;
			return DECLINED; //ERROR;
		}
		// status_codeのみ
		if (elms.size() == 1) {
			std::cout << "status_code only" << std::endl;
			return DECLINED; //OK;
		}
		path = elms[1];
	}
//	std::cout << "path(aft): " << path << std::endl;
	redirect_path = path;
    // needs path with support status_code
	// compile_complex_valueは$の展開をしてそう
    return status_code;
//	return OK;
}

static std::string createMtime(time_t modified)
{
    char buf[1000];
    //time_t now = time(0);
    struct tm tm = *gmtime(&modified);
    std::strftime(buf, sizeof(buf), "%d-%b-%Y %H:%M ", &tm);
    std::string str(buf);
    return str;
}


std::string HttpRes::create_auto_index_html(std::map<std::string, dir_t> index_of) {
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
        body += "\">"; //
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


std::string HttpRes::join_path_autoindex() {
//    std::cout << "===== join_path =====" << std::endl;
	std::string path_root = target.get_root();
	std::string config_path  = target.get_uri();
	std::string file_path = httpreq.getUri().substr(config_path.length());
	/*
						|            request uri       |
		/User/root/path/ /config/location/ /file_path.html
		path_root         config_path     file_path
	*/
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
		if (file_path.size() >= 1) { //
			file_path = file_path.substr(1);
        }
	}
	std::cout << "auto index join_path: " << path_root + config_path + file_path << std::endl;
//    std::cout << "===== End join_path =====" << std::endl;
	return path_root + config_path + file_path;
}


int HttpRes::auto_index_handler() {
    std::cout << "================== auto_index_handler ==================" << std::endl;
//    std::cout << "keep-alive: " << httpreq.getKeepAlive() << std::endl;
    std::string req_uri = httpreq.getUri();
    target = get_uri2location(req_uri);
    if (req_uri[req_uri.length() - 1] != '/' || !(target.get_is_autoindex())) {
//        std::cout << "ko" << std::endl;
        return DECLINED;
    }
	std::string method = httpreq.getMethod();
	if (method != "GET" && method != "HEAD") {
        std::cerr << "not allow method in auto_index handler" << std::endl;
		return DECLINED;
	}

	std::vector<std::string> allow_methods = target.get_methods();
	if (find(allow_methods.begin(), allow_methods.end(), method) == allow_methods.end()) {
		status_code = BAD_REQUEST; // or NOT_ALLOWED
		return DECLINED;
	}
    // discard req body

//    std::string dir_path = join_path();
    std::string dir_path = join_path_autoindex();
//    std::cout << "dir_path: " << dir_path << std::endl;
    if (dir_path[dir_path.length() - 1] == '/') {
        dir_path = dir_path.substr(0, dir_path.length() - 1);
    }
//    std::cout << "dir_path: " << dir_path.c_str() << std::endl;
    dir_t dir_info;
    dir_info.dir = opendir(dir_path.c_str());
    if (dir_info.dir == NULL) {
       if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG) {
           std::cout << "NOT_FOUND" << std::endl;
           return NOT_FOUND;
       } else if (errno == EACCES) {
           std::cout << "FORBIDDEN" << std::endl;
           return FORBIDDEN;
       }
//       std::cout << "errno: " << errno << std::endl;
       std::cout << "INTERNAL_SERVER_ERROR" << std::endl;
       return INTERNAL_SERVER_ERROR;
    }
    status_code = HTTP_OK; // 200
    // auto_index only text/html for now
    content_type = "text/html";
    sendHeader(); // later ?

    dir_info.valid_info = 0;
    std::map<std::string, dir_t> index_of;
    for (;;) {
        errno = 0;
        dir_info.d_ent = readdir(dir_info.dir);
        if (!dir_info.d_ent) {
            if (errno != 0) {
                std::cerr << "readdir Error" << std::endl;
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
        std::string abs_path = join_dir_path(dir_path, file_name);
        if (!dir_info.valid_info) {
            if (stat(abs_path.c_str(), &(dir_info.d_info)) == -1) {
                //only EACCES
                continue;
            }
            // handle ENOENT or ELOOP -> error or INTERNAL_SERVER_ERROR
        }
        // check link info

        index_of[file_name] = dir_info;
    }
    if (closedir(dir_info.dir) == -1) {
        std::cerr << "closedir Error" << std::endl;
    }
    out_buf = create_auto_index_html(index_of);
    body_size = out_buf.length();
//    std::cout << "auot_index_html: " << out_buf << std::endl;
    return OK;

}

bool HttpRes::is_cgi() {
	Location location = get_uri2location(httpreq.getUri()); //req uri?
//    std::cout << "Location: " << location << std::endl;
//    std::cout << "loc cgi: " << location.get_cgi_ext() << std::endl;
//	if (location.get_cgi_path() != "") {
    std::vector<std::string> vec = location.get_cgi_ext();
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
//    std::cout << "=== no cgi ===" << std::endl;
	return false;
}


void HttpRes::runHandlers() {
	int handler_status = 0;
	if (is_cgi()) {
        std::cout << "================== cgi ==================" << std::endl;
	    Location location = get_uri2location(httpreq.getUri()); //req uri?
        httpreq.set_meta_variables(location);
		Cgi cgi(httpreq ,location);
		cgi.run_cgi();
        handler_status = cgi.parse_cgi_response();
//		std::cout << "resType: " << cgi.getResType() << std::endl;
        if (cgi.getResType() == DOCUMENT) {
            status_code = handler_status;
            cgi.getHeaderFields().erase("Status");
            set_cgi(cgi);
//            std::cout << cgi.buf << std::endl;
            sendHeader(); //tmp here
            out_buf = cgi.getCgiBody();
            if (cgi.getHeaderFields().count("Content-Length")) {
                std::stringstream ss(cgi.getHeaderFields()["Content-Length"]);
                ss >> body_size;
            } else {
                body_size = out_buf.length();
            }

            return finalize_res(status_code);
        } else if (cgi.getResType() == LOCAL_REDIRECT) {
			if (httpreq.isRedirectLimit()) {
//				std::cerr << "cnt" << std::endl;
                status_code = 500;
                return finalize_res(status_code);
			}
            httpreq.setUri(cgi.getHeaderFields()["Location"]);
			httpreq.incrementRedirectCnt();
            return runHandlers();
        } else if (cgi.getResType() == CLIENT_REDIRECT || cgi.getResType() == CLIENT_REDIRECT_WITH_DOC) {
            status_code = 302;
			redirect_path = cgi.getHeaderFields()["Location"];
			body = cgi.getCgiBody();
			header_filter();

			return finalize_res(status_code);
//        std::cout << cgi.buf << std::endl;
        }
	} else {
//  	  static int i = 0;
		handler_status = return_redirect();
		if (handler_status != DECLINED) {
			return finalize_res(handler_status);
		}
    	handler_status = static_handler();
    	if (handler_status != DECLINED) {
//    	    std::cout << "in finalize" << std::endl;
    	    return finalize_res(handler_status);
    	}
    	handler_status = auto_index_handler();
    	if (handler_status != DECLINED) {
//    	    std::cout << "in finalize" << std::endl;
    	    return finalize_res(handler_status);
    	}
//  	  std::cout << "run handler i: " << i++ << std::endl;
    	//std::cout << "handler status after static handler: " << handler_status << std::endl;
		handler_status = dav_delete_handler();
        if (handler_status != DECLINED) {
            return finalize_res(handler_status);
        }
	}
}
