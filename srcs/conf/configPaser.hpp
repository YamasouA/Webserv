#ifndef CONFIGPASER_HPP
#define CONFIGPASER_HPP

#include "virtualServer.hpp"
#include "Location.hpp"

class configPaser {
	public:
		explicit configPaser(const std::string& strs);
		configPaser(const configPaser& src);
		configPaser& operator=(const configPaser& rhs);
		~configPaser();
		void configParser::parseConf();
	private:
		std::string buf;
		std::vector<virtualServer> serve_confs;
		size_t idx;

		void configPaser::parseServe(size_t i);
		Location parseLocation();
		std::string configPaser::getToken(char delimiter);
		std::string get_token_to_eol();
};

static std::vector<std::string> methodsSplit(std::string strs, char delimi);
void skip();
const std::string readConfFile(const std::string& file_name);
#endif