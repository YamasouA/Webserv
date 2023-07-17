#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include "httpReq.hpp"
#include "../conf/Location.hpp"

class Cgi {
    public:
        Cgi(const httpReq& request, Location location);
        Cgi(const Cgi& src);
        Cgi& operator=(const Cgi& rhs);
        ~Cgi();

        void run_cgi();
        char buf[1024];
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
        int parse_cgi_response();

        httpReq httpreq;
        Location target;
        std::map<std::string, std::string> envs; // or sep all var
        std::map<std::string, std::string> header_fields;
};

#endif
