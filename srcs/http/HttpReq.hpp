#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <algorithm>
#include <string>
#include <limits>
#include <vector>
#include <map>
#include <iostream>
#include <ostream>
#include <sstream>
#include <iomanip>
#include "../conf/Location.hpp"

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
	HTTP_VERSION_NOT_SUPPORTED = 505,
	HTTP_INSUFFICIENT_STORAGE = 507
};

class HttpReq {
    public:
        HttpReq();
		HttpReq(const std::string& request_msg);
        HttpReq(const HttpReq& src);
        HttpReq& operator=(const HttpReq& rhs);
        ~HttpReq();

        void		setClientIP(std::string client_ip);
        void		setPort(int port);
        void		setMethod(const std::string&);
        void		setUri(const std::string&);
        void		setVersion(const std::string&);
        void		setContentBody(const std::string&);
		void		setHeaderField(const std::string& name, const std::string value);
        void		setMetaVariables(Location loc);
        void		setErrStatus(int err_status);

        std::string getClientIP() const;
        int			getPort() const;
        std::string getMethod() const;
        std::string getUri() const;
        std::string getVersion() const;
        std::string getContentBody() const;
        std::string getBuf() const;
        size_t		getContentLength() const;
		std::string getQueryString() const;
        int			getKeepAlive() const;
        int			getRedirectCnt() const;
        int			getErrStatus() const;
        std::map<std::string, std::string> getHeaderFields() const;
        std::map<std::string, std::string> get_meta_variables() const;

		bool		isRedirectLimit();
		void		incrementRedirectCnt();
		void		appendReq(char *str);
		void		parseHeader();
		void		parseBody();
		bool		isEndOfHeader() const;
		bool		isEndOfReq() const;

    private:
		static const int kRedirectLimit = 10;
		static const int kMaxPortNum = 65535;
		static const int kMaxUriLength = 8000;

		static const int RE_RECV = 1;
		static const int OK = 0;
		static const int ERROR = -1;

        size_t		idx;
		int			redirect_cnt;
		bool		is_header_end;
		bool		is_req_end;
        int			keep_alive;
        size_t		content_length;
        int			err_status;
		bool		is_in_chunk_data;
		size_t		chunk_size;
        int			port;
        std::string client_ip;
		std::string body_buf;
        std::string buf;
        std::string method;
        std::string uri;
        std::string args;
        std::string version;
        std::string content_body;
		std::string query_string;
        std::map<std::string, std::string> header_fields;
        std::map<std::string, std::string> cgi_envs;

        void		skipEmptyLines();
		void		skipSpace();
		int			expect(char c);
		std::string getToken(char delimiter);
		std::string getUriToken(char delimiter);
		std::string getTokenToEOL();
		void		parseReqLine();
		bool		checkHeaderEnd();
		std::string getTokenToEOF();
		void		checkUri();
		void		parseScheme();
		void		parseHostPort();
		void		fixUp();
		void		absUrlParse();
		void		parseAuthorityAndPath();
		void		parseChunk();
		std::string percentEncode();
		void		appendHeader(std::string str);
		void		appendBody(std::string str);
		int			getChunkSize();
		int			getChunkData();
		void		skipTokenToEOF();
		size_t		getChunkedSize() const;
		bool		isInChunkData() const;
		void		rejectReq(int err_status);
};

std::ostream& operator<<(std::ostream& stream, const HttpReq& obj);

#endif
