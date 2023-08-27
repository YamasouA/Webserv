#include "cgi.hpp"

Cgi::Cgi() {}

Cgi::Cgi(const httpReq& request, Location location)
:httpreq(request),
    target(location),
	status(200),
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

int Cgi::getStatusCode() const {
    return status;
}

std::string Cgi::joinPath() {
    std::cerr << "===== joinPath(cgi) =====" << std::endl;
	std::string path_root = target.getRoot();

	std::string config_path  = target.getUri();
    std::string script_name = envs["SCRIPT_NAME"];
	std::string alias;
	if ((alias = target.getAlias()) != "") {
		config_path = alias;
	}
	if ((path_root.size() && path_root[path_root.length() - 1] == '/') || path_root.size() == 0) {
		if (config_path.size() >= 1)
			config_path = config_path.substr(1);
	}
	if (config_path == "" || config_path[config_path.length() - 1] == '/') {
		if (script_name.size() >= 1) {
			script_name = script_name.substr(1);
        }
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
		status = 502;
    }
    if (envs.count("REMOTE_ADDR") == 0) {
		status = 502;
    }
    if (envs.count("REMOTE_HOST") == 0) {
		status = 502;
    }
    if (envs.count("REQUEST_METHOD") == 0) {
		status = 502;
    }
    if (envs.count("SCRIPT_NAME") == 0) {
		status = 502;
    }
    if (envs.count("SERVER_NAME") == 0) {
		status = 502;
    }
    if (envs.count("SERVER_PORT") == 0) {
		status = 502;
    }
    if (envs.count("SERVER_PROTOCOL") == 0) {
		status = 502;
    }
    if (envs.count("SERVER_SOFTWARE") == 0) {
		status = 502;
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

void Cgi::sendBodyToChild() {
    if (httpreq.getContentBody().length() > 0) {
	    write(1, httpreq.getContentBody().c_str(), httpreq.getContentBody().length());
    }
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
    std::string path = joinPath();
	if (execve(path.c_str(), NULL, envs_ptr) < 0) {
        std::cerr << "failed exec errno: " << errno << std::endl;
    }
}

void Cgi::forkProcess() {
	pid_t pid;
	int fd[2];
	int fd2[2];


	if (pipe(fd) == -1) {
        status = 502;
        return;
    }
	if (pipe(fd2) == -1) {
        status = 502;
        return;
    }
	setEnv();

	if (status != 200)
		return;
	pid = fork();
    if (pid == -1) {
        status = 502;
        return;
    }
	if (pid == 0) {
//		set_signal_handler(SIGINT, SIG_DFL);
//		set_signal_handler(SIGQUIT, SIG_DFL);
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
        status = 502;
        return;
    }
	if (dup2(fd2[0], 0) == -1) {
        status = 502;
        return;
    }
	sendBodyToChild();
	pid_t pid2 = 0;
	int st = 0;
	while (pid2 != -1) {
		pid2 = waitpid(-1, &st, 0);
		if (!WIFEXITED(st)) {
			status = 502;
		}
	}
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

std::string Cgi::getTokenToEOL(size_t& idx) {
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

int Cgi::parseCgiResponse() {
    size_t idx = 0;
	if (status != 200)
		return status;

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
        std::string field_value = getTokenToEOL(idx);
		std::cout << "field_value: " << field_value<< std::endl;
        trim(field_value);
        setHeaderField(toLower(field_name), field_value);
    }
    cgi_body = getTokenToEOF(idx);
    fixUp();
    return status;
}

//void	Cgi::get_exit_status(pid_t pid)
//{
//	int		exit_status;
//	pid_t	pid2;
//
//	pid2 = 0;
//	while (pid2 != -1)
//	{
//		pid2 = waitpid(-1, &exit_status, 0);
//		if (pid == pid2)
//		{
//			if (WIFSIGNALED(exit_status))
//			{
////				if (WTERMSIG(status) == SIGQUIT)
//                status = 502;
////				status = WTERMSIG(status);
////                std::cout << "exit status" << status << std::endl;
//			}
////			else if (WIFEXITED(exit_status))
////				exit_status = WEXITSTATUS(exit_status);
//		}
//	}
//}


void Cgi::runCgi() {

	int backup_stdin = dup(STDIN_FILENO);
    if (backup_stdin == -1) {
        status = 502;
        return;
    }
	int backup_stdout = dup(STDOUT_FILENO);
    if (backup_stdout == -1) {
        status = 502;
        return;
    }

	forkProcess();

	if (dup2(backup_stdin, STDIN_FILENO) == -1) {
        status = 502;
        return;
    }
    if (dup2(backup_stdout, STDOUT_FILENO) == -1) {
        status = 502;
        return;
    }
	close(backup_stdin);
	close(backup_stdout);
//    get_exit_status(pid);
}
