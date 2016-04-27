#pragma once

#include <string>
#include <vector>

struct Options {
	std::string take;
	std::vector<std::string> mounts;
	int threads;
	bool progressive;
};

namespace options {

void init(int argc, char* argv[]);

const Options& options();

}
