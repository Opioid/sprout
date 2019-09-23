#ifndef SU_BASE_MATH_SCALAR_HPP
#define SU_BASE_MATH_SCALAR_HPP

#include "simd/simd.hpp"

namespace math {

struct Simd3f;

//==============================================================================
// SIMD float scalar
//==============================================================================

struct Simd1f {
    Simd1f() noexcept;

    Simd1f(__m128 m) noexcept;

    explicit Simd1f(float s) noexcept;

    float x() const noexcept;

    __m128 v;
};

}  // namespace math

#endif
