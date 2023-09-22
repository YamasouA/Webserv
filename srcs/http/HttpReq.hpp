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

class HttpReq {
    public:
        HttpReq();
		HttpReq(const std::string& request_msg);
        HttpReq(const HttpReq& src);
        HttpReq& operator=(const HttpReq& rhs);
        ~HttpReq();

        void setClientIP(std::string client_ip);
        void setPort(int port);
        void setMethod(const std::string&);
        void setUri(const std::string&);
        void setVersion(const std::string&);
        void setContentBody(const std::string&);
		void setHeaderField(const std::string& name, const std::string value);
        void setMetaVariables(Location loc);
        void setErrStatus(int err_status);

        std::string getClientIP() const;
        int getPort() const;
        std::string getMethod() const;
        std::string getUri() const;
        std::string getVersion() const;
        std::string getContentBody() const;
        std::string getBuf() const;
        size_t getContentLength() const;
		std::string getQueryString() const;
        std::map<std::string, std::string> getHeaderFields() const;
        int getKeepAlive() const;
        std::map<std::string, std::string> get_meta_variables() const;
        int getRedirectCnt() const;
        int getErrStatus() const;
		bool isRedirectLimit();
		void incrementRedirectCnt();
		void appendReq(char *str);
		void parseHeader();
		void parseBody();
		bool isEndOfHeader() const;
		bool isEndOfReq() const;
    private:
        std::string body_buf;
        std::string buf;
        size_t idx;
        std::string client_ip;
        int port;
		int redirect_cnt;
		static const int kRedirectLimit = 10;

		static const int RE_RECV = 1;
		static const int OK = 0;
		static const int ERROR = -1;
		bool is_header_end;
		bool is_req_end;

        std::string method;
        std::string uri;
        std::string args;
        std::string version;
        std::map<std::string, std::string> header_fields;
        std::map<std::string, std::string> cgi_envs;
        std::string content_body;
        int keep_alive;
		std::string query_string;
        size_t content_length;
        int err_status;
		size_t chunk_size;
		bool is_in_chunk_data;

        void skipEmptyLines();
		void skipSpace();
		int expect(char c);
		std::string getToken(char delimiter);
		std::string getUriToken(char delimiter);
		std::string getTokenToEOL();
		void parseReqLine();
		bool checkHeaderEnd();
		std::string getTokenToEOF();
		void checkUri();
		void parseScheme();
		void parseHostPort();
		void fixUp();
		void absUrlParse();
		void parseAuthorityAndPath();
		void parseChunk();
		std::string percentEncode();
		void appendHeader(std::string str);
		void appendBody(std::string str);
		int getChunkSize();
		int getChunkData();
		void skipTokenToEOF();
		size_t getChunkedSize() const;
		bool isInChunkData() const;
		void rejectReq(int err_status);
};

std::ostream& operator<<(std::ostream& stream, const HttpReq& obj);

#endif