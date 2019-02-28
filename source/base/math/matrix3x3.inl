#ifndef SU_BASE_MATH_MATRIX3X3_INL
#define SU_BASE_MATH_MATRIX3X3_INL

#include <cmath>
#include "matrix3x3.hpp"
#include "vector4.inl"

namespace math {
/*
template<typename T>
Matrix3x3<T>::Matrix3x3() {}

template<typename T>
Matrix3x3<T>::Matrix3x3(T m00, T m01, T m02,
                                                T m10, T m11, T m12,
                                                T m20, T m21, T m22):
        m00(m00), m01(m01), m02(m02),
        m10(m10), m11(m11), m12(m12),
        m20(m20), m21(m21), m22(m22) {}

template<typename T>
Matrix3x3<T>::Matrix3x3(Vector3<T> const& x, Vector3<T> const& y, Vector3<T> const& z) :
        x(x), y(y), z(z) {}

template<typename T>
Matrix3x3<T>::Matrix3x3(const Matrix4x4<T>& m) :
        m00(m.m00), m01(m.m01), m02(m.m02),
        m10(m.m10), m11(m.m11), m12(m.m12),
        m20(m.m20), m21(m.m21), m22(m.m22) {}

template<typename T>
Matrix3x3<T> Matrix3x3<T>::operator*(const Matrix3x3& m) const {
        return Matrix3x3(m00 * m.m00 + m01 * m.m10 + m02 * m.m20,
                                         m00 * m.m01 + m01 * m.m11 + m02 * m.m21,
                                         m00 * m.m02 + m01 * m.m12 + m02 * m.m22,

                                         m10 * m.m00 + m11 * m.m10 + m12 * m.m20,
                                         m10 * m.m01 + m11 * m.m11 + m12 * m.m21,
                                         m10 * m.m02 + m11 * m.m12 + m12 * m.m22,

                                         m20 * m.m00 + m21 * m.m10 + m22 * m.m20,
                                         m20 * m.m01 + m21 * m.m11 + m22 * m.m21,
                                         m20 * m.m02 + m21 * m.m12 + m22 * m.m22);
}

template<typename T>
Matrix3x3<T> Matrix3x3<T>::operator/(T s) const {
        T is = T(1) / s;
        return Matrix3x3<T>(is * m00, is * m01, is * m02,
                                                is * m10, is * m11, is * m12,
                                                is * m20, is * m21, is * m22);
}

template<typename T>
Matrix3x3<T>& Matrix3x3<T>::operator*=(const Matrix3x3& m) {
        Matrix3x3 temp(m00 * m.m00 + m01 * m.m10 + m02 * m.m20,
                                   m00 * m.m01 + m01 * m.m11 + m02 * m.m21,
                                   m00 * m.m02 + m01 * m.m12 + m02 * m.m22,

                                   m10 * m.m00 + m11 * m.m10 + m12 * m.m20,
                                   m10 * m.m01 + m11 * m.m11 + m12 * m.m21,
                                   m10 * m.m02 + m11 * m.m12 + m12 * m.m22,

                                   m20 * m.m00 + m21 * m.m10 + m22 * m.m20,
                                   m20 * m.m01 + m21 * m.m11 + m22 * m.m21,
                                   m20 * m.m02 + m21 * m.m12 + m22 * m.m22);

        return *this = temp;
}

template<typename T>
Matrix3x3<T> Matrix3x3<T>::identity() {
        return Matrix3x3<T>(T(1), T(0), T(0),
                                                T(0), T(1), T(0),
                                                T(0), T(0), T(1));
}

template<typename T>
Vector3<T> operator*(Vector3<T> const& v, const Matrix3x3<T>& m) {
        return Vector3<T>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20,
                                          v.x * m.m01 + v.y * m.m11 + v.z * m.m21,
                                          v.x * m.m02 + v.y * m.m12 + v.z * m.m22);
}

template<typename T>
Vector3<T>& operator*=(Vector3<T>& v, const Matrix3x3<T>& m) {
        Vector3<T> temp(v.x * m.m00 + v.y * m.m10 + v.z * m.m20,
                                        v.x * m.m01 + v.y * m.m11 + v.z * m.m21,
                                        v.x * m.m02 + v.y * m.m12 + v.z * m.m22);

        return v = temp;
}

template<typename T>
Vector3<T> transform_vector(const Matrix3x3<T>& m, Vector3<T> const& v) {
        return Vector3<T>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20,
                                          v.x * m.m01 + v.y * m.m11 + v.z * m.m21,
                                          v.x * m.m02 + v.y * m.m12 + v.z * m.m22);
}

template<typename T>
Vector3<T> transform_vector_transposed(const Matrix3x3<T>& m, Vector3<T> const& v) {
        return Vector3<T>(v.x * m.m00 + v.y * m.m01 + v.z * m.m02,
                                          v.x * m.m10 + v.y * m.m11 + v.z * m.m12,
                                          v.x * m.m20 + v.y * m.m21 + v.z * m.m22);
}

template<typename T>
void transform_vectors(const Matrix3x3<T>& m,
                                           Vector3<T> const& a, Vector3<T> const& b, Vector3<T>& oa,
Vector3<T>& ob) { oa.x = a.x * m.m00 + a.y * m.m10 + a.z * m.m20; oa.y = a.x * m.m01 + a.y * m.m11 +
a.z * m.m21; oa.z = a.x * m.m02 + a.y * m.m12 + a.z * m.m22;

        ob.x = b.x * m.m00 + b.y * m.m10 + b.z * m.m20;
        ob.y = b.x * m.m01 + b.y * m.m11 + b.z * m.m21;
        ob.z = b.x * m.m02 + b.y * m.m12 + b.z * m.m22;
}

template<typename T>
Matrix3x3<T> normalize(const Matrix3x3<T>& m) {
        T s0 = T(1) / length(Vector3<T>(m.m00, m.m01, m.m02));
        T s1 = T(1) / length(Vector3<T>(m.m10, m.m11, m.m12));
        T s2 = T(1) / length(Vector3<T>(m.m20, m.m21, m.m22));

        return Matrix3x3<T>(s0 * m.m00, s0 * m.m01, s0 * m.m02,
                                                s1 * m.m10, s1 * m.m11, s1 * m.m12,
                                                s2 * m.m20, s2 * m.m21, s2 * m.m22);
}

template<typename T>
T det(const Matrix3x3<T>& m) {
        return m.m00 * m.m11 * m.m22
                 + m.m10 * m.m21 * m.m02
                 + m.m20 * m.m01 * m.m12
                 - m.m00 * m.m21 * m.m12
                 - m.m20 * m.m11 * m.m02
                 - m.m10 * m.m01 * m.m22;
}

template<typename T>
Matrix3x3<T> invert(const Matrix3x3<T>& m) {
        return Matrix3x3<T>(m.m11 * m.m22 - m.m12 * m.m21, m.m02 * m.m21 - m.m01 * m.m22, m.m01 *
m.m12 - m.m02 * m.m11, m.m12 * m.m20 - m.m10 * m.m22, m.m00 * m.m22 - m.m02 * m.m20, m.m02 * m.m10 -
m.m00 * m.m12, m.m10 * m.m21 - m.m11 * m.m20, m.m01 * m.m20 - m.m00 * m.m21, m.m00 * m.m11 - m.m01 *
m.m10) / det(m);
}

template<typename T>
void set_look_at(Matrix3x3<T>& m, Vector3<T> const& dir, Vector3<T> const& up) {
        Vector3<T> z = normalize(dir);
        Vector3<T> x = normalize(cross(up, z));
        Vector3<T> y = cross(z, x);

//	m.m00 =  x.x;		  m.m01 = y.x;		    m.m02 = z.x;
//	m.m10 =  x.y;         m.m11 = y.y;          m.m12 = z.y;
//	m.m20 =  x.z;         m.m21 = y.z;          m.m22 = z.z;

        //Confusion!

        m.m00 =  x.x;		  m.m01 = x.y;		    m.m02 = x.z;
        m.m10 =  y.x;         m.m11 = y.y;          m.m12 = y.z;
        m.m20 =  z.x;         m.m21 = z.y;          m.m22 = z.z;
}

//template<typename T>
//void set_basis(Matrix3x3<T>& m, Vector3<T> const& v) {
//	m.rows[2] = v;

//	if (v.x < T(0.6) && v.x > -T(0.6)) {
//		m.rows[1] = Vector3<T>(T(1), T(0), T(0));
//	} else if (v.y < T(0.6) && v.y > T(0.6)) {
//		m.rows[1] = Vector3<T>(T(0), T(1), T(0));
//	} else {
//		m.rows[1] = Vector3<T>(T(0), T(0), T(1));
//	}

//	m.rows[0] = normalize(cross(v, m.rows[1]));
//	m.rows[1] = cross(m.rows[0], m.rows[2]);
//}

template<typename T>
void set_scale(Matrix3x3<T>& m, T x, T y, T z) {
        m.m00 = x;    m.m01 = T(0); m.m02 = T(0);
        m.m10 = T(0); m.m11 = y;    m.m12 = T(0);
        m.m20 = T(0); m.m21 = T(0); m.m22 = z;
}

template<typename T>
void set_scale(Matrix3x3<T>& m, Vector3<T> const& v) {
        m.m00 = v.x;  m.m01 = T(0); m.m02 = T(0);
        m.m10 = T(0); m.m11 = v.y;  m.m12 = T(0);
        m.m20 = T(0); m.m21 = T(0); m.m22 = v.z;
}

template<typename T>
void scale(Matrix3x3<T>& m, Vector3<T> const& v) {
        m.m00 *= v.x; m.m01 *= v.x; m.m02 *= v.x;
        m.m10 *= v.y; m.m11 *= v.y; m.m12 *= v.y;
        m.m20 *= v.z; m.m21 *= v.z; m.m22 *= v.z;
}

template<typename T>
void set_rotation_x(Matrix3x3<T>& m, T a) {
        T c = std::cos(a);
        T s = std::sin(a);

        m.m00 = T(1); m.m01 = T(0); m.m02 = T(0);
        m.m10 = T(0); m.m11 = c;    m.m12 = -s;
        m.m20 = T(0); m.m21 = s;    m.m22 =  c;
}

template<typename T>
void set_rotation_y(Matrix3x3<T>& m, T a) {
        T c = std::cos(a);
        T s = std::sin(a);

        m.m00 =  c;    m.m01 = T(0); m.m02 = s;
        m.m10 =  T(0); m.m11 = T(1); m.m12 = T(0);
        m.m20 = -s;    m.m21 = T(0); m.m22 = c;
}

template<typename T>
void set_rotation_z(Matrix3x3<T>& m, T a) {
        T c = std::cos(a);
        T s = std::sin(a);

        m.m00 = c;    m.m01 = -s;    m.m02 = T(0);
        m.m10 = s;    m.m11 =  c;    m.m12 = T(0);
        m.m20 = T(0); m.m21 =  T(0); m.m22 = T(1);
}

template<typename T>
void set_rotation(Matrix3x3<T>& m, Vector3<T> const& v, T a) {
        T c = std::cos(a);
        T s = std::sin(a);
        T t = T(1) - c;

        m.m00 = c + v.x * v.x * t;
        m.m11 = c + v.y * v.y * t;
        m.m22 = c + v.z * v.z * t;

        T tmp1 = v.x * v.y * t;
        T tmp2 = v.z * s;

        m.m10 = tmp1 + tmp2;
        m.m01 = tmp1 - tmp2;

        tmp1 = v.x * v.z * t;
        tmp2 = v.y * s;

        m.m20 = tmp1 - tmp2;
        m.m02 = tmp1 + tmp2;

        tmp1 = v.y * v.z * t;
        tmp2 = v.x * s;

        m.m21 = tmp1 + tmp2;
        m.m12 = tmp1 - tmp2;
}

template<typename T>
Matrix3x3<T> transposed(const Matrix3x3<T>& m) {
        return Matrix3x3<T>(m.m00, m.m10, m.m20,
                                                m.m01, m.m11, m.m21,
                                                m.m02, m.m12, m.m22);
}
*/
//==============================================================================
// Aligned 3x3 float matrix
//==============================================================================

inline Matrix3x3f_a::Matrix3x3f_a() noexcept = default;

inline constexpr Matrix3x3f_a::Matrix3x3f_a(float m00, float m01, float m02, float m10, float m11,
                                            float m12, float m20, float m21, float m22) noexcept
    : r{{m00, m01, m02}, {m10, m11, m12}, {m20, m21, m22}} {}

inline constexpr Matrix3x3f_a::Matrix3x3f_a(Vector3f_a const& x, Vector3f_a const& y,
                                            Vector3f_a const& z) noexcept
    : r{x, y, z} {}

inline Matrix3x3f_a constexpr Matrix3x3f_a::identity() noexcept {
    return Matrix3x3f_a(1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f);
}

static inline Matrix3x3f_a constexpr operator+(Matrix3x3f_a const& a,
                                               Matrix3x3f_a const& b) noexcept {
    return Matrix3x3f_a(a.r[0][0] + b.r[0][0], a.r[0][1] + b.r[0][1], a.r[0][2] + b.r[0][2],
                        a.r[1][0] + b.r[1][0], a.r[1][1] + b.r[1][1], a.r[1][2] + b.r[1][2],
                        a.r[2][0] + b.r[2][0], a.r[2][1] + b.r[2][1], a.r[2][2] + b.r[2][2]);
}

static inline Matrix3x3f_a constexpr operator*(float s, Matrix3x3f_a const& b) noexcept {
    return Matrix3x3f_a(s * b.r[0][0], s * b.r[0][1], s * b.r[0][2], s * b.r[1][0], s * b.r[1][1],
                        s * b.r[1][2], s * b.r[2][0], s * b.r[2][1], s * b.r[2][2]);
}

static inline Matrix3x3f_a operator*(Matrix3x3f_a const& a, Matrix3x3f_a const& b) noexcept {
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

static inline Vector3f_a transform_vector(Matrix3x3f_a const& m, Vector3f_a const& v) noexcept {
    return Vector3f_a(v[0] * m.r[0][0] + v[1] * m.r[1][0] + v[2] * m.r[2][0],
                      v[0] * m.r[0][1] + v[1] * m.r[1][1] + v[2] * m.r[2][1],
                      v[0] * m.r[0][2] + v[1] * m.r[1][2] + v[2] * m.r[2][2]);
}

static inline Vector3f_a transform_vector_transposed(Matrix3x3f_a const& m,
                                                     Vector3f_a const&   v) noexcept {
    return Vector3f_a(v[0] * m.r[0][0] + v[1] * m.r[0][1] + v[2] * m.r[0][2],
                      v[0] * m.r[1][0] + v[1] * m.r[1][1] + v[2] * m.r[1][2],
                      v[0] * m.r[2][0] + v[1] * m.r[2][1] + v[2] * m.r[2][2]);
}

static inline void set_rotation_x(Matrix3x3f_a& m, float a) noexcept {
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

static inline void set_rotation_y(Matrix3x3f_a& m, float a) noexcept {
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

static inline void set_rotation_z(Matrix3x3f_a& m, float a) noexcept {
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

static inline void set_rotation(Matrix3x3f_a& m, Vector3f_a const& v, float a) noexcept {
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

static inline Matrix3x3f_a constexpr lerp(Matrix3x3f_a const& a, Matrix3x3f_a const& b,
                                          float t) noexcept {
    float const u = 1.f - t;
    return u * a + t * b;
}

static inline Matrix3x3f_a constexpr clamp(Matrix3x3f_a const& m, float mi, float ma) {
    return Matrix3x3f_a(
        std::min(std::max(m.r[0][0], mi), ma), std::min(std::max(m.r[0][1], mi), ma),
        std::min(std::max(m.r[0][2], mi), ma), std::min(std::max(m.r[1][0], mi), ma),
        std::min(std::max(m.r[1][1], mi), ma), std::min(std::max(m.r[1][2], mi), ma),
        std::min(std::max(m.r[2][0], mi), ma), std::min(std::max(m.r[2][1], mi), ma),
        std::min(std::max(m.r[2][2], mi), ma));
}

}  // namespace math

#endif
