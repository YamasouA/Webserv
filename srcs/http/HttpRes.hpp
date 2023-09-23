#ifndef HTTPRES_HPP
#define HTTPRES_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include "../conf/Location.hpp"
#include "../conf/VirtualServer.hpp"
#include "HttpReq.hpp"
#include "Cgi.hpp"
#include "../Logger.hpp"

enum server_state {
    OK = 0,
    DECLINED = -1 //tmp value
};

typedef struct {
    DIR             *dir;
    struct dirent   *d_ent;
    struct stat     d_info;
    bool            valid_info:1;
} dir_t;

class Client;

class HttpRes {
	private:
		static const std::string kServerName;
		static const std::string default_type;

		Logger logger;
		std::string body; //maybe unnecessary, but use in cgiHnadler

		int				status_code;
		size_t			content_length_n;
		bool			header_only;
        int				keep_alive;
        int				err_status;
		bool			is_sended_header;
		bool			is_sended_body;
        size_t			header_size;
        size_t			body_size;

		int redirect_cnt;

		std::string		status_line;
		std::string		content_type;
        std::string		charset;
        std::string		location_field;
		std::string		buf;
        std::string		out_buf;

		HttpReq			httpreq;
		VirtualServer	vServer;
        Cgi				cgi;
		Location		target;

		bool isRedirectLimit();
		void incrementRedirectCnt();
        std::string createDate(std::string fieldName);
		int setContentType();
		void headerFilter();
		void createStatusLine();
		void addAllowField();
		void addContentTypeField();
		void addContentLengthField();
		void addConnectionField();
		void addLocationField();

        int staticHandler();
        void sendHeader();
        Location getUri2Location(std::string uri) const;

		int deleteHandler();
		int deletePath(bool is_dir);
        int deleteError();
        std::string joinDirPath(const std::string& dir_path, const std::string& elem_name);
        void divingThroughDir(const std::string& path);

		void finalizeRes(int handler_status);
        std::string createErrPage();
        int redirectHandle();
		int returnRedirect();
        int sendErrorPage();
        int autoindexHandler();
        std::string createAutoIndexHtml(std::map<std::string, dir_t> index_of);
		bool isCgi();
        std::string joinPath();
        std::string joinPathAutoindex();

        int checkClientBodySize();
        void cgiHandler();
        void httpHandler();
		int HandleSafeMethod(const char *file_name, std::string& uri);
		int handlePost(std::string& file_name);
		int checkAccessToPOST(const char *file_name);
		int checkAccessToGET(const char *file_name, const std::string& uri);
		int createDestFile(std::string& file_name);
		int handleResBody(const std::string& file_name);
		int opendirError();
	public:
        HttpRes();
        HttpRes(const HttpRes& src);
		HttpRes(const Client& source);
		HttpRes& operator=(const HttpRes& rhs);
		~HttpRes();

        Cgi			getCgi() const;
        size_t		getHeaderSize() const;
        std::string getResBody() const;
        size_t		getBodySize() const;
		bool		getIsSendedBody() const;
		bool		getIsSendedHeader() const;
		std::string getLocationField() const;
		int			getKeepAlive() const;
        std::string getBuf() const;
        void		setBody(std::string strs);
        void		setCgi(Cgi cgi);
		void		setIsSendedHeader(bool b);
		void		setIsSendedBody(bool b);
		void		setLocationField(std::string loc);
		bool		isHeaderOnly() const;
        void		runHandlers();
        void		handleReqErr(int req_err_status);

		std::string redirect_path;
};

#endif
