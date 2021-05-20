#ifndef SU_BASE_MATH_SCALAR_HPP
#define SU_BASE_MATH_SCALAR_HPP

#include "simd.hpp"

namespace math {

struct Scalar {
    Scalar();

    Scalar(__m128 m);

    explicit Scalar(float s);

    float x() const;

    __m128 v;
};

}  // namespace math

#endif
