#ifndef VIRTUALSERVER_HPP
#define VIRTUALSERVER_HPP

#include <vector>
#include <map>
#include <string>
#include "Location.hpp"

class virtualServer {
	public:
		virtualServer();
		virtualServer(const virtualServer& src);
		virtualServer& operator=(const virtualServer& rhs);
		~virtualServer();

		void parse_vserv();
		void set_listen(int listen);
		void set_server_name(std::vector<std::string> server_name);
		void set_location(Location location);
		void set_root(std::string root);
		std::vector<Location> get_locations() const;
        std::vector<int> get_listen() const;
        std::vector<std::string> get_server_names() const;
		std::string get_root() const;
		std::map<std::string, Location> get_uri2location() const;
		void set_uri2location(std::map<std::string, Location> uri2location);


        void set_methods(std::vector<std::string> methods);
		void set_is_autoindex(bool autoindex);
		void set_upload_path(std::string upload_path);
		void set_index(std::vector<std::string> index);
		void set_max_body_size(size_t max_body_size);
		void set_cgi_path(std::string cgi_path);
		void set_return(std::string ret);
		void set_depth(int depth);
		void set_alias(std::string alias);
		void set_error_pages(std::vector<std::string> tokens);
		void set_cgi_ext(std::vector<std::string> tokens);

		std::vector<std::string> get_methods() const;
		bool get_is_autoindex() const;
		std::string get_upload_path() const;
        std::vector<std::string> get_index() const;
		size_t get_max_body_size() const;
		std::string get_cgi_path() const;
		std::string get_return() const;
		std::string get_error_page(int status_code) const;
		std::map<int, std::string> get_error_pages() const;
		int get_depth() const;
		std::string get_alias() const;
		std::vector<std::string> get_cgi_ext() const;

        void append_index(std::vector<std::string> elems);
        void append_cgi_ext(std::vector<std::string> elems);
	private:
        std::vector<int> listen;
        std::vector<std::string> server_names;
		std::string root;
		std::vector<Location> locations;
		std::string path;
		std::string error_page;
		std::map<std::string, Location> uri2location;

        int whichOneExist;

        std::vector<std::string> index;
		std::string ret;
		std::vector<std::string> methods;
		std::string upload_path;
		std::string cgi_path;
		std::map<int, std::string > error_pages;
		int depth;
		std::string alias;
		bool autoindex;
		size_t max_body_size;
		std::vector<std::string> cgi_ext;
};

std::ostream& operator <<(std::ostream& stream, const virtualServer& obj);
#endif
