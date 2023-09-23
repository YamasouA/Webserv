#include "Cgi.hpp"

Cgi::Cgi() {}

Cgi::Cgi(const HttpReq& request, Location location)
:httpreq(request),
    target(location),
	resType(NO_MATCH_TYPE),
	status(200),
    envs(request.get_meta_variables())
{}

Cgi::Cgi(const Cgi& src)
:target(src.target),
	resType(NO_MATCH_TYPE),
	status(src.status),
	envs(src.envs),
	header_fields(src.getHeaderFields())
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

int Cgi::getStatusCode() const {
    return status;
}

void Cgi::setStatusCode(int status) {
	this->status = status;
}

std::string Cgi::joinPath() {
    std::cerr << "===== joinPath(cgi) =====" << std::endl;
	std::string path_root = target.getRoot();
	if (path_root == "" || path_root[0] != '/') {
		path_root = "./" + path_root;
	}

	std::string config_path  = target.getUri();
    std::string script_name = envs["SCRIPT_NAME"];
	if (config_path.length() < script_name.length()) {
		script_name = script_name.substr(config_path.length());
	} else {
		script_name = "";
	}
	std::string alias;
	if ((alias = target.getAlias()) != "") {
		config_path = alias;
	}
	if ((path_root.size() && path_root[path_root.length() - 1] == '/') || path_root.size() == 0) {
		if (config_path.size() >= 1)
			config_path = config_path.substr(1);
	}
	std::cerr << "joinPath: " << path_root + config_path + script_name << std::endl;
    std::cerr << "===== End joinPath =====" << std::endl;
	return path_root + config_path + script_name;
}


void Cgi::envsFixUp() {
	if (envs.count("CONTENT_LENGTH") == 0) {
		envs["CONTENT_LENGTH"] = "0";
	}
    if (envs.count("CONTENT_TYPE") == 0 && httpreq.getContentBody().length() > 0) {
		envs["CONTENT_TYPE"] = "application/octet-stream";
    }
    if (envs.count("GETAWAY_INTERFACE") == 0) {
		envs["GATEWAY_INTERFACE"] = "CGI/1.1";
    }
    if (envs.count("PATH_INFO") != envs.count("PATH_TRANSLATED")) {
		// PATH_INFOとPATH_TRANSLATEDはどちらも存在するかどちらも存在しない
		setStatusCode(INTERNAL_SERVER_ERROR);
    }
    if (envs.count("REMOTE_ADDR") == 0) {
		setStatusCode(INTERNAL_SERVER_ERROR);
    }
    if (envs.count("REMOTE_HOST") == 0) {
		setStatusCode(INTERNAL_SERVER_ERROR);
    }
    if (envs.count("REQUEST_METHOD") == 0) {
		setStatusCode(INTERNAL_SERVER_ERROR);
    }
    if (envs.count("SCRIPT_NAME") == 0) {
		setStatusCode(INTERNAL_SERVER_ERROR);
    }
    if (envs.count("SERVER_NAME") == 0) {
		setStatusCode(INTERNAL_SERVER_ERROR);
    }
    if (envs.count("SERVER_PORT") == 0) {
		setStatusCode(INTERNAL_SERVER_ERROR);
    }
    if (envs.count("SERVER_PROTOCOL") == 0) {
		setStatusCode(INTERNAL_SERVER_ERROR);
    }
    if (envs.count("SERVER_SOFTWARE") == 0) {
		setStatusCode(INTERNAL_SERVER_ERROR);
    }
	envs.erase("HTTP_HOST");
	envs.erase("HTTP_CONTENT_LENGTH");
	envs.erase("HTTP_CONTENT_TYPE");
}

void Cgi::setEnv() {
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
	envsFixUp();
}

ssize_t Cgi::sendBodyToChild() {
	ssize_t cnt = 0;
    if (httpreq.getContentBody().length() > 0) {
	    cnt = write(1, httpreq.getContentBody().c_str(), httpreq.getContentBody().length());
    }
	return cnt;
}

void Cgi::runHandler() {
	char **envs_ptr;
    std::map<std::string, std::string>::iterator its = envs.begin();
    for (; its != envs.end(); ++its) {
        std::cerr << its->first << "=" << its->second << std::endl;;
    }

	envs_ptr = new char *[envs.size() + 1];
	std::map<std::string, std::string>::iterator it = envs.begin();
    std::vector<std::string> tmp_vec;
	size_t i = 0;
	for (; it != envs.end(); ++it) {
		std::string env_exp = it->first + "=" + it->second;
        tmp_vec.push_back(env_exp);
    }
    for (std::vector<std::string>::iterator vec_it = tmp_vec.begin(); vec_it != tmp_vec.end(); ++vec_it) {
        envs_ptr[i] = (char*)vec_it->c_str();
		i++;
    }
	envs_ptr[envs.size()] = 0;
    std::string path = joinPath();
	char *argv[2];
	argv[0] = new char[path.size() + 1];
	std::strcpy(argv[0], path.c_str());
	argv[1] = NULL;
	if (execve(path.c_str(), argv, envs_ptr) < 0) {
        std::cerr << "failed exec errno: " << errno << std::endl;
		delete [] envs_ptr;
		delete [] argv[0];
    }
}

void Cgi::forkProcess() {
	pid_t pid;
	int fd[2];
	int fd2[2];


	if (pipe(fd) == -1) {
		return setStatusCode(INTERNAL_SERVER_ERROR);
    }
	if (pipe(fd2) == -1) {
		return setStatusCode(INTERNAL_SERVER_ERROR);
    }
	setEnv();

	if (status != 200)
		return;
	pid = fork();
    if (pid == -1) {
		return setStatusCode(INTERNAL_SERVER_ERROR);
    }
	if (pid == 0) {
		close(fd[1]);
		close(fd2[0]);

		if (dup2(fd[0], 0) == -1) {
            std::exit(1);
        }
        if (dup2(fd2[1], 1) == -1) {
            std::exit(1);
        }
		runHandler();
        std::exit(1);
	}
	close(fd[0]);
	close(fd2[1]);
	if (dup2(fd[1], 1) == -1) {
		return setStatusCode(INTERNAL_SERVER_ERROR);
    }
	if (dup2(fd2[0], 0) == -1) {
		return setStatusCode(INTERNAL_SERVER_ERROR);
    }
	if (sendBodyToChild() <= 0 && httpreq.getContentBody().length() > 0) {
		kill(pid, SIGKILL);
		return setStatusCode(INTERNAL_SERVER_ERROR);
	}
	pid_t pid2 = 0;
	int st = 0;
	time_t before_wait = std::time(NULL);
	while (pid2 != -1) {
		pid2 = waitpid(pid, &st, WNOHANG);
		if (WIFEXITED(st) && WEXITSTATUS(st) != 0) {
			setStatusCode(INTERNAL_SERVER_ERROR);
		} else {
			std::exit(1);
		}
		if (std::time(NULL) - before_wait >= 3) {
			kill(pid, SIGKILL);
			setStatusCode(HTTP_GATEWAY_TIME_OUT);
			break;
		}
	}
	close(fd[1]);
    char tmp_buf;
    while (read(0, &tmp_buf, 1) > 0 && status != HTTP_GATEWAY_TIME_OUT) {
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

static void trim(std::string& str)
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

std::string Cgi::getTokenToEOL(size_t& idx) {
	std::string line = "";
	while (idx < buf.length()) {
		if (buf[idx] == '\015') {
			if (buf[idx+1] == '\012') {
				idx += 2;
				return line;
			} else {
				setStatusCode(INTERNAL_SERVER_ERROR);
				return "";
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

std::string Cgi::getTokenToEOF(size_t& idx) {
	std::string body = "";
	while (idx < buf.length()) {
		body += buf[idx];
		idx++;
	}
	return body;
}

void Cgi::fixUp() {
    if (header_fields.size() == 0) {
		return setStatusCode(HTTP_BAD_GATEWAY);
    }
    if (header_fields.count("status") == 1) {
		// ここもutil関数使いたい
        std::stringstream ss;
        ss << header_fields["status"];
        ss >> status;
        if (status < 100 || 600 <= status) {
			setStatusCode(HTTP_BAD_GATEWAY);
        }
		header_fields.erase("status");
    }
    detectResType();
	if (resType == NO_MATCH_TYPE)
		setStatusCode(HTTP_BAD_GATEWAY);
}

// util関数
static std::string toLower(std::string str) {
	std::string s = "";
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

int Cgi::parseCgiResponse() {
    size_t idx = 0;
	if (status != HTTP_OK)
		return status;

	while (idx < buf.length()) {
        if (checkHeaderEnd(idx)) {
            break;
        }
        std::string field_name = getToken(':', idx);
		if (field_name == "") {
			setStatusCode(HTTP_BAD_GATEWAY);
			std::cout << "cgi_body: " << cgi_body << std::endl;
			std::map<std::string, std::string>::iterator it = header_fields.begin();
			for (; it != header_fields.end(); it++) {
				std::cout << it->first << ": " << it->second << std::endl;
			}
			return status;
		}
		std::cout << "field_name: " << field_name << std::endl;
        skipSpace(idx);
        std::string field_value = getTokenToEOL(idx);
		std::cout << "field_value: " << field_value<< std::endl;
        trim(field_value);
        setHeaderField(toLower(field_name), field_value);
    }
    cgi_body = getTokenToEOF(idx);
    fixUp();
    return status;
}

void Cgi::runCgi() {

	int backup_stdin = dup(STDIN_FILENO);
    if (backup_stdin == -1) {
		return setStatusCode(INTERNAL_SERVER_ERROR);
    }
	int backup_stdout = dup(STDOUT_FILENO);
    if (backup_stdout == -1) {
		return setStatusCode(INTERNAL_SERVER_ERROR);
    }


    std::string path = joinPath();
	if (access(path.c_str(), R_OK) < 0) {
		if (errno == EACCES) {
			return setStatusCode(FORBIDDEN);
		} else {
			return setStatusCode(NOT_FOUND);
		}
	}

	forkProcess();

	if (dup2(backup_stdin, STDIN_FILENO) == -1) {
		return setStatusCode(INTERNAL_SERVER_ERROR);
    }
    if (dup2(backup_stdout, STDOUT_FILENO) == -1) {
		return setStatusCode(INTERNAL_SERVER_ERROR);
    }
	close(backup_stdin);
	close(backup_stdout);
}
