#include "configParser.hpp"
#include <dirent.h>
#include <stdio.h>

static std::string join(const char *path, const char *file_name)
{
	std::string p(path);
	std::string f(file_name);
	return p + "/" + f;
}

void test(const char *dirpath) {
    DIR *dir;
    struct dirent *dp;
	dir = opendir(dirpath);

	if (dir == NULL) { return; }
	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_name[0] == '.') {
			continue;
		}
		std::cerr << "\033[1;31m"
			<< "==========" << dp->d_name << "==========" << "\033[0m"
			<< std::endl;
		try {
			std::string txt = readConfFile(join(dirpath, dp->d_name));
			configParser confParser(txt);
			confParser.parseConf();
			std::vector<virtualServer> conf_server = confParser.getServerConfs();
			for (std::vector<virtualServer>::iterator it = conf_server.begin();
				it != conf_server.end(); it++) {
				std::cout << *it << std::endl;
			}
		} catch (const std::exception& e) {
			std::cout << e.what() << std::endl;
		}
	}
}

int main(int argc, char **argv) {
    if (argc == 1)
    {
		std::cerr << "\033[1;32m"
			<< "==========invalid==========" << "\033[0m"
			<< std::endl;
		test("invalid_test");
		std::cerr << "\033[1;32m"
			<< "==========valid==========" << "\033[0m"
			<< std::endl;
		test("valid_test");
    }
    else {
		test(argv[1]);
    }
}
