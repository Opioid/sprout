#ifndef SU_BASE_MATH_MATRIX3X3_HPP
#define SU_BASE_MATH_MATRIX3X3_HPP

#include "matrix.hpp"
#include "vector3.hpp"

namespace math {

struct alignas(16) Matrix3x3f_a {
    Vector3f_a r[3];

    Matrix3x3f_a();

    constexpr Matrix3x3f_a(float m00, float m01, float m02, float m10, float m11, float m12,
                           float m20, float m21, float m22);

    explicit constexpr Matrix3x3f_a(Vector3f_a_p x, Vector3f_a_p y, Vector3f_a_p z);

    static Matrix3x3f_a constexpr identity();
};

struct alignas(16) Simd3x3f {
    Simd3f r[3];

    Simd3x3f(Matrix3x3f_a const& source);
};

}  // namespace math

#endif
