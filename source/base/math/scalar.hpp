#ifndef SU_BASE_MATH_SCALAR_HPP
#define SU_BASE_MATH_SCALAR_HPP

#include "simd/simd.hpp"

namespace math {

struct Simd1f {
    Simd1f();

    Simd1f(__m128 m);

    explicit Simd1f(float s);

    float x() const;

    __m128 v;
};

}  // namespace math

#endif
