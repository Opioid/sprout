#ifndef SU_DI_OPTIONS_OPTIONS_HPP
#define SU_DI_OPTIONS_OPTIONS_HPP

#include <string>
#include <vector>

namespace di::options {

struct Options {
    std::string reference;

    std::vector<std::string> images;

    int threads = 0;
};

Options parse(int argc, char* argv[]) noexcept;

}  // namespace di::options

#endif
