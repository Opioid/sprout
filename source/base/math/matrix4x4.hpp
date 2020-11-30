#ifndef SU_BASE_MATH_MATRIX4X4_HPP
#define SU_BASE_MATH_MATRIX4X4_HPP

#include "matrix.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

namespace math {

struct Transformation;

struct alignas(16) Matrix4x4f_a {
    Vector4f_a r[4];

    Matrix4x4f_a();

    Matrix4x4f_a(float m00, float m01, float m02, float m03, float m10, float m11, float m12,
                 float m13, float m20, float m21, float m22, float m23, float m30, float m31,
                 float m32, float m33);

    explicit Matrix4x4f_a(float const* a);

    explicit Matrix4x4f_a(Matrix3x3f_a const& m);

    explicit Matrix4x4f_a(Transformation const& t);

    Vector3f_a x() const;
    Vector3f_a y() const;
    Vector3f_a z() const;
    Vector3f_a w() const;
};

struct alignas(16) Simd4x4f {
    Simd3f r[4];

    Simd4x4f(Matrix4x4f_a const& source);
};

}  // namespace math

#endif
