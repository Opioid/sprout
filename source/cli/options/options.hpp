#pragma once

#include <string>
#include <vector>

struct Options {
	std::string take;
	std::vector<std::string> mounts;
	int threads = 0;
	bool progressive = false;
	bool no_textures = false;
	bool verbose = false;
};

namespace options {

Options parse(int argc, char* argv[]);

}
