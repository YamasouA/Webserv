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
		std::vector<virtualServer> getServerConfs()const;
		void fixUp();
		void checkServer();
		void checkLocation();
		void set_buf(std::string strs);
	private:
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

		virtualServer parseServe();
		void setUriToMap(std::string prefix, std::string prefix_root, Location location, const virtualServer& v_serv);
		void uriToMap(virtualServer& vServer);
		Location parseLocation();
		std::string getToken(char delimiter);
		std::string getTokenToEOL();
		std::map<std::string, Location> uri2location;

		void handleListenInServ(virtualServer& v_serv);
//		void handleServerNameInServ(virtualServer& v_serv, int *which_one_exist);
		void handleRootInServ(virtualServer& v_serv, int *which_one_exist);
		void handleIndexInServ(virtualServer& v_serv, int *which_one_exist);
		void handleReturnInServ(virtualServer& v_serv, int *which_one_exist);
		void handleMethodInServ(virtualServer& v_serv, int *which_one_exist);
		void handleAutoindexInServ(virtualServer& v_serv, int *which_one_exist);
		void handleUploadPathInServ(virtualServer& v_serv);
//		void handleUploadPathInServ(virtualServer& v_serv, int *which_one_exist);
		void handleMaxBodySizeInServ(virtualServer& v_serv, int *which_one_exist);
//		void handleLocationInServ(virtualServer& v_serv, int *which_one_exist);
		void handleErrorPageInServ(virtualServer& v_serv);
		void handleCgiExtInServ(virtualServer& v_serv, int *which_one_exist);

		void handleRootInLoc(Location& location, int *which_one_exist);
		void handleIndexInLoc(Location& location, int *which_one_exist);
		void handleReturnInLoc(Location& location, int *which_one_exist);
		void handleMethodInLoc(Location& location, int *which_one_exist);
		void handleAutoindexInLoc(Location& location, int *which_one_exist);
		void handleUploadPathInLoc(Location& location, int *which_one_exist);
		void handleMaxBodySizeInLoc(Location& location, int *which_one_exist);
		void handleAliasInLoc(Location& location, int *which_one_exist);
//		void handleLocationInLoc(Location& location, int *which_one_exist);
		void handleErrorPageInLoc(Location& location, int *which_one_exist);
		void handleCgiExtInLoc(Location& location, int *which_one_exist);
		class SyntaxException: public std::exception {
			public:
				explicit SyntaxException(const std::string& what_arg);
				~SyntaxException() throw();
				virtual const char* what() const throw();
			private:
				std::string msg;
		};
		class DupulicateException: public std::exception {
			public:
				explicit DupulicateException(const std::string& what_arg);
				~DupulicateException() throw();
				virtual const char* what() const throw();
			private:
				std::string msg;
		};
		class ConfigValueException: public std::exception {
			public:
				explicit ConfigValueException(const std::string& what_arg);
				~ConfigValueException() throw();
				virtual const char* what() const throw();
			private:
				std::string msg;
		};
};

const std::string readConfFile(const std::string& file_name);
#endif
