#ifndef SU_CLI_OPTIONS_OPTIONS_HPP
#define SU_CLI_OPTIONS_OPTIONS_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace options {

struct Options {
    std::string take;

    std::vector<std::string> mounts;

    int32_t threads = 0;

    uint32_t start_frame = 0;
    uint32_t num_frames  = 1;

    bool no_tex         = false;
    bool no_tex_dwim    = false;
    bool debug_material = false;
    bool quit           = false;
};

Options parse(int argc, char* argv[]);

}  // namespace options

#endif
