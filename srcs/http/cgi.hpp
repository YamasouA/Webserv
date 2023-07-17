#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include "httpReq.hpp"
#include "../conf/Location.hpp"

class Cgi {
    public:
        Cgi();
        Cgi(const httpReq& request, Location location);
        Cgi(const Cgi& src);
        Cgi& operator=(const Cgi& rhs);
        ~Cgi();

        void run_cgi();
        std::map<std::string, std::string> getHeaderFields() const;
        int parse_cgi_response();
        std::string buf;
    private:
        void fork_process();
        void run_handler();
        void send_body_to_child();
        void set_env();
        void fix_up();
        std::string encode_uri();
        bool check_meta_var(std::string var1, std::string var2);
        std::string join_path();
//        std::string join_path(std::string& script_name);
//        int parse_cgi_response();

        httpReq httpreq;
        Location target;
		std::string cgi_body;
        std::map<std::string, std::string> envs; // or sep all var
        std::map<std::string, std::string> header_fields;
		void fixUp(int& status);
		std::string getToken_to_eof(size_t& idx);
		std::string getToken_to_eol(size_t& idx);
		std::string getToken(char delimiter, size_t& idx);
		bool checkHeaderEnd(size_t& idx);
		void skipSpace(size_t& idx);
		void setHeaderField(const std::string& name, const std::string value);
		void trim(std::string& str);
		void expect(char c, size_t& idx);
		class SyntaxException: public std::exception {
			public:
				explicit SyntaxException(const std::string& what_arg);
				~SyntaxException() throw();
				virtual const char* what() const throw(); // throw() = noexcept
			private:
				std::string msg;
		};
};

#endif
