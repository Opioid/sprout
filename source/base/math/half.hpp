#ifndef SU_BASE_MATH_HALF_HPP
#define SU_BASE_MATH_HALF_HPP

#include <cstdint>

namespace math {

static int16_t float_to_half(float f) noexcept;

struct half {
    half(float s) noexcept;

    int16_t h;
};

}  // namespace math

#endif
