#ifndef SU_IT_OPTIONS_OPTIONS_HPP
#define SU_IT_OPTIONS_OPTIONS_HPP

#include <string>
#include <vector>

namespace it::options {

struct Options {
    bool diff = true;

    float clamp = std::numeric_limits<float>::max();
    float clip  = std::numeric_limits<float>::max();

    std::vector<std::string> images;

    int threads = 0;
};

Options parse(int argc, char* argv[]) noexcept;

}  // namespace it::options

#endif
