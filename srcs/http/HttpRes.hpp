#ifndef HTTPRES_HPP
#define HTTPRES_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include "../conf/Location.hpp"
#include "../conf/VirtualServer.hpp"
#include "HttpReq.hpp"
#include "Cgi.hpp"

enum server_state {
    OK = 0,
    DECLINED = -1 //tmp value
};

enum Status {
	HTTP_OK = 200,
	CREATED = 201,
	NO_CONTENT =204,
	PARTIAL_CONTENT = 206,

	SPECIAL_RESPONSE = 300,
	MOVED_PERMANENTLY = 301,
	MOVED_TEMPORARILY = 302,
	NOT_MODIFIED = 304,

	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	NOT_ALLOWED = 405,
	REQUEST_TIME_OUT = 408,
	CONFLICT = 409,
	LENGTH_REQUIRED = 411,
	PRECONDITION_FAILED = 412,
	REQUEST_ENTITY_TOO_LARGE = 413,
	REQUEST_URI_TOO_LARGE = 414,
	UNSUPPORTED_MEDIA_TYPE = 415,
	RANGE_NOT_SATISFIABLE = 416,

	INTERNAL_SERVER_ERROR = 500,
	HTTP_NOT_IMPLEMENTED = 501,
	HTTP_BAD_GATEWAY = 502,
	HTTP_SERVICE_UNAVAILABLE = 503,
	HTTP_GATEWAY_TIME_OUT = 504,
	HTTP_INSUFFICIENT_STORAGE = 507
};

typedef struct {
    DIR             *dir;
    struct dirent   *d_ent;
    struct stat     d_info;
    bool            valid_info:1;
} dir_t;

class Client;

class HttpRes {
	private:
		static const std::string kServerName;
		std::string header;
		std::string body;

		size_t header_idx;
		size_t body_idx;
		int status_code;
		std::string status_string;
		std::string status_line;
		size_t content_length_n;
		std::string content_type;
		static const std::string default_type;
		bool is_posted;
		std::string location;
		bool header_only;
        std::string charset;
        int keep_alive;

        int err_status;

        std::string location_field;

		HttpReq httpreq;
		VirtualServer vServer;
//        Kqueue* connection;
		int fd;
        Cgi cgi;

		bool is_sended_header;
		bool is_sended_body;

		Location target;

		std::string buf;
        size_t header_size;
        std::string out_buf;
        size_t body_size;

		void createResponseHeader(struct stat sb);
		std::string getStatusString();
		void createControlData();
        std::string createDate(std::string fieldName);
		void createContentLength();
		int setContentType();
		void headerFilter();
		void createStatusLine();
		void addAllowField();
		void addContentTypeField();
		void addContentLengthField();
		void addConnectionField();
		void addLocationField();

        int staticHandler();
        void sendHeader();
        Location getUri2Location(std::string uri) const;

		int deleteHandler();
		int deletePath(bool is_dir);
		int dav_depth();
        int deleteError();
        std::string joinDirPath(const std::string& dir_path, const std::string& elem_name);
        void divingThroughDir(const std::string& path);
        void finalizeRes(int handler_status);
        std::string createErrPage();
        int redirectHandle();
		int returnRedirect();
        int sendErrorPage();
        int autoindexHandler();
        std::string createAutoIndexHtml(std::map<std::string, dir_t> index_of);
		bool isCgi();
        std::string joinPathAutoindex();

        int checkClientBodySize();
        void cgiHandler();
        void httpHandler();
		int HandleSafeMethod(const char *file_name, std::string& uri);
		int handlePost(std::string& file_name);
		int checkAccessToPOST(const char *file_name);
		int checkAccessToGET(const char *file_name, const std::string& uri);
		int createDestFile(std::string& file_name);
		int handleResBody(const std::string& file_name);
		int opendirError();
	public:
        HttpRes();
        HttpRes(const HttpRes& src);
//		HttpRes(const Client& source, Kqueue &kq);
		HttpRes(const Client& source);
		HttpRes& operator=(const HttpRes& rhs);
		~HttpRes();
        std::string joinPath();
        void setBody(std::string strs);
        void setCgi(Cgi cgi);
        Cgi getCgi() const;
		void createResponse();
        void runHandlers();
        void handleReqErr(int req_err_status);
        std::string getBuf() const;
        size_t getHeaderSize() const;
        std::string getResBody() const;
        size_t getBodySize() const;
//		Kqueue* getConnection() const;
		std::string redirect_path;

		void setIsSendedHeader(bool b);
		void setIsSendedBody(bool b);
		bool getIsSendedBody() const;
		bool getIsSendedHeader() const;
		void createErrorResponse(int status);
		void setLocationField(std::string loc);
    std::string getLocationField() const;
		int getKeepAlive() const;
		bool isHeaderOnly() const;
};

#endif
