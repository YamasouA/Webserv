#ifndef CGI_HPP
#define CGI_HPP

#include <cstdlib>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include "HttpReq.hpp"
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
        Cgi(const HttpReq& request, Location location);
        Cgi(const Cgi& src);
        Cgi& operator=(const Cgi& rhs);
        ~Cgi();

        void runCgi();
        std::map<std::string, std::string> getHeaderFields() const;
        std::string getCgiBody() const;
        int getResType() const;
        int parseCgiResponse();

        int getStatusCode() const;

    private:
        HttpReq httpreq;
        Location target;
        std::string buf;
		std::string cgi_body;
        int resType;
		int status;

        void forkProcess();
        void runHandler();
        void sendBodyToChild();
        void setEnv();
        void envsFixUp();
        std::string encodeUri();
        bool checkMetaVar(std::string var1, std::string var2);
        std::string joinPath();
		void setStatusCode(int status);

        std::map<std::string, std::string> envs;
        std::map<std::string, std::string> header_fields;
		void fixUp();
		std::string getTokenToEOF(size_t& idx);
		std::string getTokenToEOL(size_t& idx);
		std::string getToken(char delimiter, size_t& idx);
		bool checkHeaderEnd(size_t& idx);
		void skipSpace(size_t& idx);
		void setHeaderField(const std::string& name, const std::string value);
		bool expect(char c, size_t& idx);

        bool isLocalRedirect();
        bool isClientRedirect();
        void detectResType();
};

#endif