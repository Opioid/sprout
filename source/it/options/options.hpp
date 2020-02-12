#ifndef SU_IT_OPTIONS_OPTIONS_HPP
#define SU_IT_OPTIONS_OPTIONS_HPP

#include "base/math/vector2.hpp"

#include <string>
#include <vector>

namespace it::options {

struct Options {
    enum class Operator { Add, Average, Cat, Diff, Sub, Undefined };

    Operator op = Operator::Undefined;

    uint32_t concat_num_per_row = 0;

    float clamp = std::numeric_limits<float>::max();

    float2 clip = float2(0.f, std::numeric_limits<float>::max());

    float max_dif = 0.f;

    std::vector<std::string> images;

    std::vector<std::string> outputs;

    std::string report;

    std::string statistics;

    std::string take;

    int threads = 0;

    bool no_export = false;
};

Options parse(int argc, char* argv[]);

}  // namespace it::options

#endif
