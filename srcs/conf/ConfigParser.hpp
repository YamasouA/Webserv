#ifndef CONFIGPASER_HPP
#define CONFIGPASER_HPP

#include <fstream>
#include <sstream>
#include <limits>
#include <map>
#include <stdexcept>
#include <exception>
#include "VirtualServer.hpp"
#include "Location.hpp"


class ConfigParser {
	public:
		explicit ConfigParser(const std::string& strs);
		explicit ConfigParser();
		ConfigParser(const ConfigParser& src);
		ConfigParser& operator=(const ConfigParser& rhs);
		~ConfigParser();

		void parseConf();
		void skip();
		void expect(char c);
		std::vector<VirtualServer> getServerConfs()const;
		void fixUp();
		void checkServer();
		void checkLocation();
		void setBuf(std::string strs);
	private:

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

		static const int kMaxPortNum = 65535;

		std::string						buf;
		size_t							idx;
		std::vector<VirtualServer>		serve_confs;
		std::map<std::string, Location> uri2location;

		std::string		getToken(char delimiter);
		std::string		getTokenToEOL();
		VirtualServer	parseServe();
		Location		parseLocation();

		void setUriToMap(std::string prefix, std::string prefix_root, Location location, const VirtualServer& v_serv);
		void uriToMap(VirtualServer& vServer);

		void handleListenInServ(VirtualServer& v_serv);
		void handleRootInServ(VirtualServer& v_serv, int *which_one_exist);
		void handleIndexInServ(VirtualServer& v_serv, int *which_one_exist);
		void handleReturnInServ(VirtualServer& v_serv, int *which_one_exist);
		void handleMethodInServ(VirtualServer& v_serv, int *which_one_exist);
		void handleAutoindexInServ(VirtualServer& v_serv, int *which_one_exist);
		void handleUploadPathInServ(VirtualServer& v_serv);
		void handleMaxBodySizeInServ(VirtualServer& v_serv, int *which_one_exist);
		void handleErrorPageInServ(VirtualServer& v_serv);
		void handleCgiExtInServ(VirtualServer& v_serv, int *which_one_exist);

		void handleRootInLoc(Location& location, int *which_one_exist);
		void handleIndexInLoc(Location& location, int *which_one_exist);
		void handleReturnInLoc(Location& location, int *which_one_exist);
		void handleMethodInLoc(Location& location, int *which_one_exist);
		void handleAutoindexInLoc(Location& location, int *which_one_exist);
		void handleUploadPathInLoc(Location& location, int *which_one_exist);
		void handleMaxBodySizeInLoc(Location& location, int *which_one_exist);
		void handleAliasInLoc(Location& location, int *which_one_exist);
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
