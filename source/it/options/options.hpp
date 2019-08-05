#ifndef SU_IT_OPTIONS_OPTIONS_HPP
#define SU_IT_OPTIONS_OPTIONS_HPP

#include <string>
#include <vector>
#include "base/math/vector2.hpp"

namespace it::options {

struct Options {
    enum class Operator { Diff, Cat, Undefined };

    Operator op = Operator::Undefined;

    uint32_t concat_num_per_row = 0;

    float clamp = std::numeric_limits<float>::max();

    float2 clip = float2(0.f, std::numeric_limits<float>::max());

    float max_dif = 0.f;

    std::vector<std::string> images;

    std::vector<std::string> outputs;

    bool no_export = false;

    std::string report;

    int threads = 0;
};

Options parse(int argc, char* argv[]) noexcept;

}  // namespace it::options

#endif
