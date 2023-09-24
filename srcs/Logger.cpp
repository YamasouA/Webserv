#include "Logger.hpp"

std::string Logger::filename = "log.txt";

void Logger::logging(std::string msg) {
	if (mkdir("logs", 0777) < 0 && errno != EEXIST) {
		std::cerr << "mkdir() Error" << std::endl;
		return;
	}
	std::string log_path;
	log_path = "logs/" + get_filename();
//	std::ofstream ofs("logs/" + get_filename(), std::ios::app);
	std::ofstream ofs(log_path.c_str(), std::ios::app);
	if (!ofs.is_open()) {
		std::cerr << "Log file can't open" << std::endl;
	}
	ofs << msg;
	ofs << std::endl;
	ofs.close();
}

std::string Logger::get_filename() {
	return filename;
}
void Logger::set_filename(std::string new_filename) {
	filename = new_filename;
}
