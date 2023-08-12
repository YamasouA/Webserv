#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <ostream>
#include <sstream>
#include <iomanip>
#include "../conf/Location.hpp"

class httpReq {
    public:
        httpReq();
        httpReq(const std::string& request_msg);
        httpReq(const httpReq& src);
        httpReq& operator=(const httpReq& rhs);
        ~httpReq();

        void setClientIP(std::string client_ip);
        void setPort(int port);
        void setMethod(const std::string&);
        void setUri(const std::string&);
        void setVersion(const std::string&);
        void setContentBody(const std::string&);
		void setHeaderField(const std::string& name, const std::string value);
        void set_meta_variables(Location loc);
        void setErrStatus(int err_status);

        std::string getClientIP() const;
        int getPort() const;
        std::string getMethod() const;
        std::string getUri() const;
        std::string getVersion() const;
        std::string getContentBody() const;
        std::string getBuf() const;
        int getContentLength() const;
		std::string getQueryString() const;
        std::map<std::string, std::string> getHeaderFields() const;
        int getKeepAlive() const;
        std::map<std::string, std::string> get_meta_variables() const;
        int getRedirectCnt() const;
        int getErrStatus() const;
		void parseRequest();
        bool isSpace(char c);
		std::string toLower(std::string str);
		bool isRedirectLimit();
		void incrementRedirectCnt();
		void appendReq(char *str);
    private:
        std::string buf;
        size_t idx;
        std::string client_ip;
        int port;
		int redirect_cnt;
		static const int kRedirectLimit = 10;

        std::string method;
        std::string uri;
        std::string args;
        std::string version;
        std::map<std::string, std::string> header_fields;
        std::map<std::string, std::string> cgi_envs;
        std::string content_body;
		bool parse_error;
        int keep_alive;
		std::string query_string;
        int content_length;
        int err_status;

        void skipEmptyLines();
		void skipSpace();
		int expect(char c);
		std::string getToken(char delimiter);
		std::string getToken_to_eol();
		void parseReqLine();
		bool checkHeaderEnd();
		std::string getToken_to_eof();
		void checkUri();
		void parse_scheme();
		void parse_host_port();
		void checkFieldsValue();
		bool hasObsFold(std::string str);
		void fix_up();
		void absurl_parse();
		void parse_authority_and_path();
		void parseChunk();
		std::string percent_encode();



		class SyntaxException: public std::exception {
			public:
				explicit SyntaxException(const std::string& what_arg);
				~SyntaxException() throw();
				virtual const char* what() const throw(); // throw() = noexcept
			private:
				std::string msg;
		};
};

std::ostream& operator<<(std::ostream& stream, const httpReq& obj);

#endif
