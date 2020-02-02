#ifndef SU_CLI_OPTIONS_OPTIONS_HPP
#define SU_CLI_OPTIONS_OPTIONS_HPP

#include <string>
#include <vector>

namespace options {

struct Options {
    std::string take;

    std::vector<std::string> mounts;

    int threads = 0;

    bool no_textures    = false;
    bool debug_material = false;
    bool quit           = false;
};

Options parse(int argc, char* argv[]);

}  // namespace options

#endif
