#ifndef LOGER_HPP
#define LOGER_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <errno.h>
class Logger {
	public:
		static void logging(std::string msg);
		static void set_filename(std::string new_filename);
		static std::string get_filename();
	private:
		static std::string filename;
};

#endif
