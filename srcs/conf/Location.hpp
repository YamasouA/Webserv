#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

class Location {
	public:
		Location();
        Location(const Location& src);
		Location& operator=(const Location& src);
		~Location();

		void						setUri(std::string uri);
		void						setMethods(std::vector<std::string> methods);
		void						setRoot(std::string root);
		void						setIsAutoindex(bool autoindex);
		void						setUploadPath(std::string upload_path);
		void						setIndex(std::vector<std::string> index);
		void						setMaxBodySize(size_t max_body_size);
		void						setCgiPath(std::string cgi_path);
		void						setReturn(std::string ret);
		void						setLocation(Location location);
		void						setAlias(std::string alias);
		void						setErrorPages(std::vector<std::string> tokens);
		void						setErrorPages(std::map<int, std::string> error_pages);
		void						setCgiExt(std::vector<std::string> tokens);
		void						setWhichOneExist(int whichOneExist);

		std::string					getUri() const;
		std::vector<std::string>	get_methods() const;
		std::string					getRoot() const;
		bool						getIsAutoindex() const;
		std::string					getUploadPath() const;
        std::vector<std::string>	getIndex() const;
		size_t						getMaxBodySize() const;
		std::string					getCgiPath() const;
		std::string					getReturn() const;
		std::string					getErrorPage(int status_code) const;
		std::map<int, std::string>	getErrorPages() const;
		std::string					getAlias() const;
		std::vector<std::string>	getCgiExt() const;
		std::vector<Location>		getLocations() const;
        int							getWhichOneExist() const;

        void appendIndex(std::vector<std::string> elems);
        void appendCgiExt(std::vector<std::string> elems);

	private:
		std::string					uri;
		std::string					root;
        std::vector<std::string>	index;
		std::string					ret;
		std::vector<std::string>	methods;
		std::string					upload_path;
		std::string					cgi_path;
		std::map<int, std::string > error_pages;
		std::string					alias;
		bool						autoindex;
		size_t						max_body_size;
		std::vector<Location>		locations;
		std::vector<std::string>	cgi_ext;
        int							which_one_exist;
};

std::ostream& operator <<(std::ostream& stream, const Location& obj);

#endif
