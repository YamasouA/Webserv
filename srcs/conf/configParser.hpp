#ifndef CONFIGPASER_HPP
#define CONFIGPASER_HPP

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <exception>
#include "virtualServer.hpp"
#include "Location.hpp"
#include <map>


class configParser {
	public:
		explicit configParser(const std::string& strs);
		explicit configParser();
		configParser(const configParser& src);
		configParser& operator=(const configParser& rhs);
		~configParser();

		void parseConf();
		void skip();
		void trim(std::string& str);
		void expect(char c);
		std::vector<virtualServer> get_serve_confs()const;
		void fixUp();
		void checkServer();
		void checkLocation();
		void set_buf(std::string strs);
	private:
//		std::map<std::string> directive_map; //neccesary?
		std::string buf;
		std::vector<virtualServer> serve_confs;
		size_t idx;

        static const int kRootExist = 1;
        static const int kAliasExist = 2;
        static const int kMaxSizeExist = 4;
        static const int kMethodExist = 8;
        static const int kAutoIndexExist = 16;
        static const int kUploadPathExist = 32;
        static const int kIndexExist = 64;
        static const int kReturnExist = 128;
        static const int kErrorPageExist = 256;
        static const int kCgiExtExist = 512;
//        int whichOneExistInServ;
//        int whichOneExistInLoc;

		//void parseServe(size_t i);
		virtualServer parseServe();
		void setUriToMap(std::string prefix, std::string prefix_root, Location location, const virtualServer& v_serv);
		void uriToMap(virtualServer& vServer);
		Location parseLocation();
		std::string getToken(char delimiter);
		std::string get_token_to_eol();
		std::map<std::string, Location> uri2location;
		// シンタックスエラー
		class SyntaxException: public std::exception {
			public:
				explicit SyntaxException(const std::string& what_arg);
				~SyntaxException() throw();
				virtual const char* what() const throw(); // throw() = noexcept
			private:
				std::string msg;
		};
		// 設定重複エラー
		class DupulicateException: public std::exception {
			public:
				explicit DupulicateException(const std::string& what_arg);
				~DupulicateException() throw();
				virtual const char* what() const throw(); // throw() = noexcept
			private:
				std::string msg;
		};
		//
		class ConfigValueException: public std::exception {
			public:
				explicit ConfigValueException(const std::string& what_arg);
				~ConfigValueException() throw();
				virtual const char* what() const throw(); // throw() = noexcept
			private:
				std::string msg;
		};
};

//static std::vector<std::string> methodsSplit(std::string strs, char delimi);
//void skip();
const std::string readConfFile(const std::string& file_name);
#endif
