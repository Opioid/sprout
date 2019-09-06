#ifndef SU_BASE_MATH_HALF_INL
#define SU_BASE_MATH_HALF_INL

#include "half.hpp"
#include "simd/simd.hpp"

namespace math {

static inline int16_t float_to_half(float f) noexcept {
    return int16_t(_mm_cvtsi128_si32(
        _mm_cvtps_ph(_mm_set_ss(f), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)));
}

inline half::half(float s) noexcept
    : h(int16_t(_mm_cvtsi128_si32(
          _mm_cvtps_ph(_mm_set_ss(s), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)))) {}

}  // namespace math

#endif
