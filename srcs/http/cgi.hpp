#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include "httpReq.hpp"
#include "../conf/Location.hpp"


enum response_types {
    NO_MATCH_TYPE = -1,
    DOCUMENT = 0,
    LOCAL_REDIRECT = 1,
    CLIENT_REDIRECT = 2,
    CLIENT_REDIRECT_WITH_DOC = 3
};

class Cgi {
    public:
        Cgi();
        Cgi(const httpReq& request, Location location);
        Cgi(const Cgi& src);
        Cgi& operator=(const Cgi& rhs);
        ~Cgi();

        void run_cgi();
        std::map<std::string, std::string> getHeaderFields() const;
        std::string getCgiBody() const;
        int getResType() const;
        int parse_cgi_response();
        std::string buf;

    private:
        void fork_process();
        void run_handler();
        void send_body_to_child();
        void set_env();
        void envs_fixUp();
        std::string encode_uri();
        bool check_meta_var(std::string var1, std::string var2);
        std::string join_path();

        httpReq httpreq;
        Location target;
		std::string cgi_body;
        int resType;
		int status;
        std::map<std::string, std::string> envs; 
        std::map<std::string, std::string> header_fields;
		void fixUp();
		std::string getToken_to_eof(size_t& idx);
		std::string getToken_to_eol(size_t& idx);
		std::string getToken(char delimiter, size_t& idx);
		bool checkHeaderEnd(size_t& idx);
		void skipSpace(size_t& idx);
		void setHeaderField(const std::string& name, const std::string value);
		void trim(std::string& str);
		bool expect(char c, size_t& idx);
		std::string toLower(std::string str);

        bool isLocalRedirect();
        bool isClientRedirect();
        void detectResType();
};

#endif
