#ifndef SU_CLI_OPTIONS_OPTIONS_HPP
#define SU_CLI_OPTIONS_OPTIONS_HPP

#include <string>
#include <vector>

namespace options {

struct Options {
    std::string              take;
    std::vector<std::string> mounts;
    int                      threads     = 0;
    bool                     progressive = false;
    bool                     no_textures = false;
    bool                     verbose     = false;
};

Options parse(int argc, const char* argv[]);

}  // namespace options

#endif
