#ifndef SU_BASE_MATH_MATRIX3X3_INL
#define SU_BASE_MATH_MATRIX3X3_INL

#include "matrix3x3.hpp"
#include "vector4.inl"

#include <cmath>

namespace math {

inline Matrix3x3f_a::Matrix3x3f_a() = default;

inline constexpr Matrix3x3f_a::Matrix3x3f_a(float m00, float m01, float m02, float m10, float m11,
                                            float m12, float m20, float m21, float m22)
    : r{{m00, m01, m02}, {m10, m11, m12}, {m20, m21, m22}} {}

inline constexpr Matrix3x3f_a::Matrix3x3f_a(Vector3f_a_p x, Vector3f_a_p y, Vector3f_a_p z)
    : r{x, y, z} {}

inline Matrix3x3f_a constexpr Matrix3x3f_a::identity() {
    return Matrix3x3f_a(1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f);
}

static inline Matrix3x3f_a constexpr operator+(Matrix3x3f_a const& a, Matrix3x3f_a const& b) {
    return Matrix3x3f_a(a.r[0][0] + b.r[0][0], a.r[0][1] + b.r[0][1], a.r[0][2] + b.r[0][2],
                        a.r[1][0] + b.r[1][0], a.r[1][1] + b.r[1][1], a.r[1][2] + b.r[1][2],
                        a.r[2][0] + b.r[2][0], a.r[2][1] + b.r[2][1], a.r[2][2] + b.r[2][2]);
}

static inline Matrix3x3f_a constexpr operator*(float s, Matrix3x3f_a const& b) {
    return Matrix3x3f_a(s * b.r[0][0], s * b.r[0][1], s * b.r[0][2], s * b.r[1][0], s * b.r[1][1],
                        s * b.r[1][2], s * b.r[2][0], s * b.r[2][1], s * b.r[2][2]);
}

static inline Matrix3x3f_a operator*(Matrix3x3f_a const& a, Matrix3x3f_a const& b) {
    return Matrix3x3f_a(a.r[0][0] * b.r[0][0] + a.r[0][1] * b.r[1][0] + a.r[0][2] * b.r[2][0],
                        a.r[0][0] * b.r[0][1] + a.r[0][1] * b.r[1][1] + a.r[0][2] * b.r[2][1],
                        a.r[0][0] * b.r[0][2] + a.r[0][1] * b.r[1][2] + a.r[0][2] * b.r[2][2],

                        a.r[1][0] * b.r[0][0] + a.r[1][1] * b.r[1][0] + a.r[1][2] * b.r[2][0],
                        a.r[1][0] * b.r[0][1] + a.r[1][1] * b.r[1][1] + a.r[1][2] * b.r[2][1],
                        a.r[1][0] * b.r[0][2] + a.r[1][1] * b.r[1][2] + a.r[1][2] * b.r[2][2],

                        a.r[2][0] * b.r[0][0] + a.r[2][1] * b.r[1][0] + a.r[2][2] * b.r[2][0],
                        a.r[2][0] * b.r[0][1] + a.r[2][1] * b.r[1][1] + a.r[2][2] * b.r[2][1],
                        a.r[2][0] * b.r[0][2] + a.r[2][1] * b.r[1][2] + a.r[2][2] * b.r[2][2]);
}

static inline Vector3f_a transform_vector(Matrix3x3f_a const& m, Vector3f_a_p v) {
    return Vector3f_a(v[0] * m.r[0][0] + v[1] * m.r[1][0] + v[2] * m.r[2][0],
                      v[0] * m.r[0][1] + v[1] * m.r[1][1] + v[2] * m.r[2][1],
                      v[0] * m.r[0][2] + v[1] * m.r[1][2] + v[2] * m.r[2][2]);
}

static inline Vector3f_a transform_vector_transposed(Matrix3x3f_a const& m, Vector3f_a_p v) {
    return Vector3f_a(v[0] * m.r[0][0] + v[1] * m.r[0][1] + v[2] * m.r[0][2],
                      v[0] * m.r[1][0] + v[1] * m.r[1][1] + v[2] * m.r[1][2],
                      v[0] * m.r[2][0] + v[1] * m.r[2][1] + v[2] * m.r[2][2]);
}

static inline void set_rotation_x(Matrix3x3f_a& m, float a) {
    float const c = std::cos(a);
    float const s = std::sin(a);

    m.r[0][0] = 1.f;
    m.r[0][1] = 0.f;
    m.r[0][2] = 0.f;
    m.r[1][0] = 0.f;
    m.r[1][1] = c;
    m.r[1][2] = -s;
    m.r[2][0] = 0.f;
    m.r[2][1] = s;
    m.r[2][2] = c;
}

static inline void set_rotation_y(Matrix3x3f_a& m, float a) {
    float const c = std::cos(a);
    float const s = std::sin(a);

    m.r[0][0] = c;
    m.r[0][1] = 0.f;
    m.r[0][2] = s;
    m.r[1][0] = 0.f;
    m.r[1][1] = 1.f;
    m.r[1][2] = 0.f;
    m.r[2][0] = -s;
    m.r[2][1] = 0.f;
    m.r[2][2] = c;
}

static inline void set_rotation_z(Matrix3x3f_a& m, float a) {
    float const c = std::cos(a);
    float const s = std::sin(a);

    m.r[0][0] = c;
    m.r[0][1] = -s;
    m.r[0][2] = 0.f;
    m.r[1][0] = s;
    m.r[1][1] = c;
    m.r[1][2] = 0.f;
    m.r[2][0] = 0.f;
    m.r[2][1] = 0.f;
    m.r[2][2] = 1.f;
}

static inline void set_rotation(Matrix3x3f_a& m, Vector3f_a_p v, float a) {
    float const c = std::cos(a);
    float const s = std::sin(a);
    float const t = 1.f - c;

    m.r[0][0] = c + v[0] * v[0] * t;
    m.r[1][1] = c + v[1] * v[1] * t;
    m.r[2][2] = c + v[2] * v[2] * t;

    float tmp1 = v[0] * v[1] * t;
    float tmp2 = v[2] * s;

    m.r[1][0] = tmp1 + tmp2;
    m.r[0][1] = tmp1 - tmp2;

    tmp1 = v[0] * v[2] * t;
    tmp2 = v[1] * s;

    m.r[2][0] = tmp1 - tmp2;
    m.r[0][2] = tmp1 + tmp2;

    tmp1 = v[1] * v[2] * t;
    tmp2 = v[0] * s;

    m.r[2][1] = tmp1 + tmp2;
    m.r[1][2] = tmp1 - tmp2;
}

static inline float determinant(Matrix3x3f_a const& m) {
    float const cofactor00 = m.r[1][1] * m.r[2][2] - m.r[1][2] * m.r[2][1];
    float const cofactor10 = m.r[1][2] * m.r[2][0] - m.r[1][0] * m.r[2][2];
    float const cofactor20 = m.r[1][0] * m.r[2][1] - m.r[1][1] * m.r[2][0];

    return m.r[0][0] * cofactor00 + m.r[0][1] * cofactor10 + m.r[0][2] * cofactor20;
}

inline Simd3x3f::Simd3x3f(Matrix3x3f_a const& source)
    : r{Simd3f(source.r[0].v), Simd3f(source.r[1].v), Simd3f(source.r[2].v)} {}

static inline Simd3f transform_vector(Simd3x3f const& m, Simd3f_p v) {
    __m128 result = SU_PERMUTE_PS(v.v, _MM_SHUFFLE(0, 0, 0, 0));
    result        = _mm_mul_ps(result, m.r[0].v);
    __m128 temp   = SU_PERMUTE_PS(v.v, _MM_SHUFFLE(1, 1, 1, 1));
    temp          = _mm_mul_ps(temp, m.r[1].v);
    result        = _mm_add_ps(result, temp);
    temp          = SU_PERMUTE_PS(v.v, _MM_SHUFFLE(2, 2, 2, 2));
    temp          = _mm_mul_ps(temp, m.r[2].v);
    result        = _mm_add_ps(result, temp);
    return result;
}

}  // namespace math

#endif
