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

		void parseVserv();
		void setListen(int listen);
		void setServerName(std::vector<std::string> server_name);
		void setLocation(Location location);
		void setRoot(std::string root);
		std::vector<Location> getLocations() const;
        std::vector<int> getListen() const;
        std::vector<std::string> getServerNames() const;
		std::string getRoot() const;
		std::map<std::string, Location> getUri2Location() const;
		void setUri2location(std::map<std::string, Location> uri2location);


        void setMethods(std::vector<std::string> methods);
		void setIsAutoindex(bool autoindex);
		void setUploadPath(std::string upload_path);
		void setIndex(std::vector<std::string> index);
		void setMaxBodySize(size_t max_body_size);
		void setCgiPath(std::string cgi_path);
		void setReturn(std::string ret);
		void setDepth(int depth);
		void setAlias(std::string alias);
		void setErrorPages(std::vector<std::string> tokens);
		void setCgiExt(std::vector<std::string> tokens);

		std::vector<std::string> get_methods() const;
		bool getIsAutoindex() const;
		std::string getUploadPath() const;
        std::vector<std::string> getIndex() const;
		size_t getMaxBodySize() const;
		std::string getCgiPath() const;
		std::string getReturn() const;
		std::string getErrorPage(int status_code) const;
		std::map<int, std::string> getErrorPages() const;
		int getDepth() const;
		std::string getAlias() const;
		std::vector<std::string> getCgiExt() const;

        void appendIndex(std::vector<std::string> elems);
        void appendCgiExt(std::vector<std::string> elems);
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
