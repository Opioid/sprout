#ifndef SU_BASE_MATH_VECTOR8_HPP
#define SU_BASE_MATH_VECTOR8_HPP

#include <cstdint>

namespace math {

//==============================================================================
// Aligned 8D unsigned short vector
//==============================================================================

struct alignas(16) Vector8us_a {
    uint16_t v[8];

    Vector8us_a() = default;

    uint16_t operator[](uint32_t i) const {
        return v[i];
    }

    uint16_t& operator[](uint32_t i) {
        return v[i];
    }
};

}  // namespace math

using ushort8 = Vector8us_a;

#endif
