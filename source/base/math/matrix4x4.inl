#ifndef SU_BASE_MATH_MATRIX4X4_INL
#define SU_BASE_MATH_MATRIX4X4_INL

#include "matrix4x4.hpp"
#include "transformation.inl"

namespace math {

/****************************************************************************
 *
 * Generic 4x4 matrix
 *
 ****************************************************************************/
/*
template<typename T>
Matrix4x4<T>::Matrix4x4() {}

template<typename T>
Matrix4x4<T>::Matrix4x4(T m00, T m01, T m02, T m03,
                                                T m10, T m11, T m12, T m13,
                                                T m20, T m21, T m22, T m23,
                                                T m30, T m31, T m32, T m33) :
        m00(m00), m01(m01), m02(m02), m03(m03),
        m10(m10), m11(m11), m12(m12), m13(m13),
        m20(m20), m21(m21), m22(m22), m23(m23),
        m30(m30), m31(m31), m32(m32), m33(m33) {}

template<typename T>
Matrix4x4<T>::Matrix4x4(const T m[16]) {
        m00 = m[0];  m01 = m[1];  m02 = m[2];  m03 = m[3];
        m10 = m[4];  m11 = m[5];  m12 = m[6];  m13 = m[7];
        m20 = m[8];  m21 = m[9];  m22 = m[10]; m23 = m[11];
        m30 = m[12]; m31 = m[13]; m32 = m[14]; m33 = m[15];
}

template<typename T>
Matrix4x4<T>::Matrix4x4(const Matrix3x3<T>& m) :
        m00(m.m00), m01(m.m01), m02(m.m02), m03(T(0)),
        m10(m.m10), m11(m.m11), m12(m.m12), m13(T(0)),
        m20(m.m20), m21(m.m21), m22(m.m22), m23(T(0)),
        m30(T(0)),  m31(T(0)),  m32(T(0)),  m33(T(1)) {}

template<typename T>
Matrix4x4<T>::Matrix4x4(Transformation const& t) {
        compose(*this, create_matrix3x3(t.rotation), t.scale, t.position);
}

template<typename T>
Matrix4x4<T> Matrix4x4<T>::operator*(const Matrix4x4& o) const {
        return Matrix4x4(m00 * o.m00 + m01 * o.m10 + m02 * o.m20 + m03 * o.m30,
                                         m00 * o.m01 + m01 * o.m11 + m02 * o.m21 + m03 * o.m31,
                                         m00 * o.m02 + m01 * o.m12 + m02 * o.m22 + m03 * o.m32,
                                         m00 * o.m03 + m01 * o.m13 + m02 * o.m23 + m03 * o.m33,

                                         m10 * o.m00 + m11 * o.m10 + m12 * o.m20 + m13 * o.m30,
                                         m10 * o.m01 + m11 * o.m11 + m12 * o.m21 + m13 * o.m31,
                                         m10 * o.m02 + m11 * o.m12 + m12 * o.m22 + m13 * o.m32,
                                         m10 * o.m03 + m11 * o.m13 + m12 * o.m23 + m13 * o.m33,

                                         m20 * o.m00 + m21 * o.m10 + m22 * o.m20 + m23 * o.m30,
                                         m20 * o.m01 + m21 * o.m11 + m22 * o.m21 + m23 * o.m31,
                                         m20 * o.m02 + m21 * o.m12 + m22 * o.m22 + m23 * o.m32,
                                         m20 * o.m03 + m21 * o.m13 + m22 * o.m23 + m23 * o.m33,

                                         m30 * o.m00 + m31 * o.m10 + m32 * o.m20 + m33 * o.m30,
                                         m30 * o.m01 + m31 * o.m11 + m32 * o.m21 + m33 * o.m31,
                                         m30 * o.m02 + m31 * o.m12 + m32 * o.m22 + m33 * o.m32,
                                         m30 * o.m03 + m31 * o.m13 + m32 * o.m23 + m33 * o.m33);
}

template<typename T>
Matrix4x4<T> Matrix4x4<T>::operator/(T s) const {
        T is = T(1) / s;
        return Matrix4x4(is * m00, is * m01, is * m02, is * m03,
                                         is * m10, is * m11, is * m12, is * m13,
                                         is * m20, is * m21, is * m22, is * m23,
                                         is * m30, is * m31, is * m32, is * m33);
}

template<typename T>
Matrix4x4<T>& Matrix4x4<T>::operator*=(const Matrix4x4& m) {
        Matrix4x4 temp(m00 * m.m00 + m01 * m.m10 + m02 * m.m20 + m03 * m.m30,
                                   m00 * m.m01 + m01 * m.m11 + m02 * m.m21 + m03 * m.m31,
                                   m00 * m.m02 + m01 * m.m12 + m02 * m.m22 + m03 * m.m32,
                                   m00 * m.m03 + m01 * m.m13 + m02 * m.m23 + m03 * m.m33,

                                   m10 * m.m00 + m11 * m.m10 + m12 * m.m20 + m13 * m.m30,
                                   m10 * m.m01 + m11 * m.m11 + m12 * m.m21 + m13 * m.m31,
                                   m10 * m.m02 + m11 * m.m12 + m12 * m.m22 + m13 * m.m32,
                                   m10 * m.m03 + m11 * m.m13 + m12 * m.m23 + m13 * m.m33,

                                   m20 * m.m00 + m21 * m.m10 + m22 * m.m20 + m23 * m.m30,
                                   m20 * m.m01 + m21 * m.m11 + m22 * m.m21 + m23 * m.m31,
                                   m20 * m.m02 + m21 * m.m12 + m22 * m.m22 + m23 * m.m32,
                                   m20 * m.m03 + m21 * m.m13 + m22 * m.m23 + m23 * m.m33,

                                   m30 * m.m00 + m31 * m.m10 + m32 * m.m20 + m33 * m.m30,
                                   m30 * m.m01 + m31 * m.m11 + m32 * m.m21 + m33 * m.m31,
                                   m30 * m.m02 + m31 * m.m12 + m32 * m.m22 + m33 * m.m32,
                                   m30 * m.m03 + m31 * m.m13 + m32 * m.m23 + m33 * m.m33);

        return *this = temp;
}

template<typename T>
const Matrix4x4<T> Matrix4x4<T>::identity = Matrix4x4<T>(T(1), T(0), T(0), T(0),
                                                                                                                 T(0), T(1), T(0), T(0),
                                                                                                                 T(0), T(0), T(1), T(0),
                                                                                                                 T(0), T(0), T(0), T(1));

template<typename T>
Matrix4x4<T> operator*(T s, const Matrix4x4<T>& m) {
        return Matrix4x4<T>(s * m.m00, s * m.m01, s * m.m02, s * m.m03,
                                                s * m.m10, s * m.m11, s * m.m12, s * m.m13,
                                                s * m.m20, s * m.m21, s * m.m22, s * m.m23,
                                                s * m.m30, s * m.m31, s * m.m32, s * m.m33);
}

template<typename T>
Vector3<T> operator*(Vector3<T> const& v, const Matrix4x4<T>& m) {
        return Vector3<T>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + m.m30,
                                          v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + m.m31,
                                          v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + m.m32);
}

template<typename T>
Vector3<T> &operator*=(Vector3<T>& v, const Matrix4x4<T>& m) {
        Vector3<T> temp(v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + m.m30,
                                        v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + m.m31,
                                        v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + m.m32);

        return v = temp;
}

template<typename T>
Vector4<T> operator*(const Vector4<T>& v, const Matrix4x4<T>& m) {
        return Vector4<T>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + v.w * m.m30,
                                          v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + v.w * m.m31,
                                          v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + v.w * m.m32,
                                          v.x * m.m03 + v.y * m.m13 + v.z * m.m23 + v.w * m.m33);
}

template<typename T>
Vector3<T> transform_vector(const Matrix4x4<T>& m, Vector3<T> const& v) {
        return Vector3<T>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20,
                                          v.x * m.m01 + v.y * m.m11 + v.z * m.m21,
                                          v.x * m.m02 + v.y * m.m12 + v.z * m.m22);
}

template<typename T>
Vector3<T> transform_point(const Matrix4x4<T>& m, Vector3<T> const& v) {
        return Vector3<T>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + m.m30,
                                          v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + m.m31,
                                          v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + m.m32);
}

template<typename T>
void get_basis(Matrix3x3<T>& basis, const Matrix4x4<T>& m) {
        basis.m00 = m.m00; basis.m01 = m.m01; basis.m02 = m.m02;
        basis.m10 = m.m10; basis.m11 = m.m11; basis.m12 = m.m12;
        basis.m20 = m.m20; basis.m21 = m.m21; basis.m22 = m.m22;
}

template<typename T>
Matrix3x3<T> extract_unscaled_basis(const Matrix4x4<T>& m) {
        return Matrix3x3<T>(normalize(m.rows[0].xyz),
                                                normalize(m.rows[1].xyz),
                                                normalize(m.rows[2].xyz));
}

template<typename T>
void set_basis(Matrix4x4<T>& m, const Matrix3x3<T>& basis) {
        m.m00 = basis.m00; m.m01 = basis.m01; m.m02 = basis.m02;
        m.m10 = basis.m10; m.m11 = basis.m11; m.m12 = basis.m12;
        m.m20 = basis.m20; m.m21 = basis.m21; m.m22 = basis.m22;
}

template<typename T>
void get_origin(Vector3<T> &origin, const Matrix4x4<T>& m)
{
        origin.x = m.m30; origin.y = m.m31; origin.z = m.m32;
}

template<typename T>
void set_origin(Matrix4x4<T>& m, Vector3<T> const& origin) {
        m.m30 = origin.x; m.m31 = origin.y; m.m32 = origin.z;
}

template<typename T>
Vector3<T> get_scale(const Matrix4x4<T>& m) {
        return Vector3<T>(length(m.rows[0].xyz), length(m.rows[1].xyz), length(m.rows[2].xyz));
}

template<typename T>
void set_scale(Matrix4x4<T>& m, T x, T y, T z) {
        m.m00 = x;    m.m01 = T(0); m.m02 = T(0); m.m03 = T(0);
        m.m10 = T(0); m.m11 = y;    m.m12 = T(0); m.m13 = T(0);
        m.m20 = T(0); m.m21 = T(0); m.m22 = z;    m.m23 = T(0);
        m.m30 = T(0); m.m31 = T(0); m.m32 = T(0); m.m33 = T(1);
}

template<typename T>
void set_scale(Matrix4x4<T>& m, Vector3<T> const& v) {
        m.m00 = v.x;  m.m01 = T(0); m.m02 = T(0); m.m03 = T(0);
        m.m10 = T(0); m.m11 = v.y;  m.m12 = T(0); m.m13 = T(0);
        m.m20 = T(0); m.m21 = T(0); m.m22 = v.z;  m.m23 = T(0);
        m.m30 = T(0); m.m31 = T(0); m.m32 = T(0); m.m33 = T(1);
}

template<typename T>
void scale(Matrix4x4<T>& m, Vector3<T> const& v) {
        m.m00 *= v.x; m.m01 *= v.x; m.m02 *= v.x;
        m.m10 *= v.y; m.m11 *= v.y; m.m12 *= v.y;
        m.m20 *= v.z; m.m21 *= v.z; m.m22 *= v.z;
}

template<typename T>
void set_translation(Matrix4x4<T>& m, T x, T y, T z) {
        m.m00 = T(1); m.m01 = T(0); m.m02 = T(0); m.m03 = T(0);
        m.m10 = T(0); m.m11 = T(1); m.m12 = T(0); m.m13 = T(0);
        m.m20 = T(0); m.m21 = T(0); m.m22 = T(1); m.m23 = T(0);
        m.m30 = x;    m.m31 = y;    m.m32 = z;    m.m33 = T(1);
}

template<typename T>
void set_translation(Matrix4x4<T>& m, Vector3<T> const& v) {
        m.m00 = T(1); m.m01 = T(0); m.m02 = T(0); m.m03 = T(0);
        m.m10 = T(0); m.m11 = T(1); m.m12 = T(0); m.m13 = T(0);
        m.m20 = T(0); m.m21 = T(0); m.m22 = T(1); m.m23 = T(0);
        m.m30 = v.x;  m.m31 = v.y;  m.m32 = v.z;  m.m33 = T(1);
}

template<typename T>
void set_rotation_x(Matrix4x4<T>& m, T a) {
        T c = std::cos(a);
        T s = std::sin(a);

        m.m00 = T(1); m.m01 = T(0); m.m02 =  T(0); m.m03 = T(0);
        m.m10 = T(0); m.m11 = c;    m.m12 = -s;    m.m13 = T(0);
        m.m20 = T(0); m.m21 = s;    m.m22 =  c;    m.m23 = T(0);
        m.m30 = T(0); m.m31 = T(0); m.m32 =  T(0); m.m33 = T(1);
}

template<typename T>
void set_rotation_y(Matrix4x4<T>& m, T a) {
        T c = std::cos(a);
        T s = std::sin(a);

        m.m00 =  c;    m.m01 = T(0); m.m02 = s;    m.m03 = T(0);
        m.m10 =  T(0); m.m11 = T(1); m.m12 = T(0); m.m13 = T(0);
        m.m20 = -s;    m.m21 = T(0); m.m22 = c;    m.m23 = T(0);
        m.m30 =  T(0); m.m31 = T(0); m.m32 = T(0); m.m33 = T(1);
}

template<typename T>
void set_rotation_z(Matrix4x4<T>& m, T a) {
        T c = std::cos(a);
        T s = std::sin(a);

        m.m00 = c;    m.m01 = -s;   m.m02 = T(0); m.m03 = T(0);
        m.m10 = s;    m.m11 = c;    m.m12 = T(0); m.m13 = T(0);
        m.m20 = T(0); m.m21 = T(0); m.m22 = T(1); m.m23 = T(0);
        m.m30 = T(0); m.m31 = T(0); m.m32 = T(0); m.m33 = T(1);
}

template<typename T>
void set_rotation(Matrix4x4<T>& m, Vector3<T> const& v, T a) {
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

        m.m03 = T(0); m.m13 = T(0); m.m23 = T(0);
        m.m30 = T(0); m.m31 = T(0); m.m32 = T(0); m.m33 = T(1);
}

template<typename T>
Matrix4x4<T> transposed(const Matrix4x4<T>& m) {
        return Matrix4x4<T>(m.m00, m.m10, m.m20, m.m30,
                                                m.m01, m.m11, m.m21, m.m31,
                                                m.m02, m.m12, m.m22, m.m32,
                                                m.m03, m.m13, m.m23, m.m33);
}

template<typename T>
T det(const Matrix4x4<T>& m) {
        return m.m00 * m.m11 * m.m22 * m.m33 + m.m00 * m.m12 * m.m23 * m.m31 + m.m00 * m.m13 * m.m21
* m.m32
                 + m.m01 * m.m10 * m.m23 * m.m32 + m.m01 * m.m12 * m.m20 * m.m33 + m.m01 * m.m13 *
m.m22 * m.m30
                 + m.m02 * m.m10 * m.m21 * m.m33 + m.m02 * m.m11 * m.m23 * m.m30 + m.m02 * m.m13 *
m.m20 * m.m31
                 + m.m03 * m.m10 * m.m22 * m.m31 + m.m03 * m.m11 * m.m21 * m.m32 + m.m03 * m.m12 *
m.m21 * m.m30

                 - m.m00 * m.m11 * m.m23 * m.m32 - m.m00 * m.m12 * m.m21 * m.m33 - m.m00 * m.m13 *
m.m22 * m.m31
                 - m.m01 * m.m10 * m.m22 * m.m33 - m.m01 * m.m12 * m.m23 * m.m30 - m.m01 * m.m13 *
m.m20 * m.m32
                 - m.m02 * m.m10 * m.m23 * m.m31 - m.m02 * m.m11 * m.m20 * m.m33 - m.m02 * m.m13 *
m.m21 * m.m30
                 - m.m03 * m.m10 * m.m21 * m.m32 - m.m03 * m.m11 * m.m22 * m.m30 - m.m03 * m.m12 *
m.m20 * m.m31;
}

template<typename T>
Matrix4x4<T> inverted(const Matrix4x4<T>& m) {
        return Matrix4x4<T>(m.m11*m.m22*m.m33 + m.m12*m.m23*m.m31 + m.m13*m.m21*m.m32 -
m.m11*m.m23*m.m32 - m.m12*m.m21*m.m33 - m.m13*m.m22*m.m31, m.m01*m.m23*m.m32 + m.m02*m.m21*m.m33 +
m.m03*m.m22*m.m31 - m.m01*m.m22*m.m33 - m.m02*m.m23*m.m31 - m.m03*m.m21*m.m32, m.m01*m.m12*m.m33 +
m.m02*m.m13*m.m31 + m.m03*m.m11*m.m32 - m.m01*m.m13*m.m32 - m.m02*m.m11*m.m33 - m.m03*m.m12*m.m31,
                                                m.m01*m.m13*m.m22 + m.m02*m.m11*m.m23 +
m.m03*m.m12*m.m21 - m.m01*m.m12*m.m23 - m.m02*m.m13*m.m21 - m.m03*m.m11*m.m22,

                                                m.m10*m.m23*m.m32 + m.m12*m.m20*m.m33 +
m.m13*m.m22*m.m30 - m.m10*m.m22*m.m33 - m.m12*m.m23*m.m30 - m.m13*m.m20*m.m32, m.m00*m.m22*m.m33 +
m.m02*m.m23*m.m30 + m.m03*m.m20*m.m32 - m.m00*m.m23*m.m32 - m.m02*m.m20*m.m33 - m.m03*m.m22*m.m30,
                                                m.m00*m.m13*m.m32 + m.m02*m.m10*m.m33 +
m.m03*m.m12*m.m30 - m.m00*m.m12*m.m33 - m.m02*m.m13*m.m30 - m.m03*m.m10*m.m32, m.m00*m.m12*m.m23 +
m.m02*m.m13*m.m20 + m.m03*m.m10*m.m22 - m.m00*m.m13*m.m22 - m.m02*m.m10*m.m23 - m.m03*m.m12*m.m20,

                                                m.m10*m.m21*m.m33 + m.m11*m.m23*m.m30 +
m.m13*m.m20*m.m31 - m.m10*m.m23*m.m31 - m.m11*m.m20*m.m33 - m.m13*m.m21*m.m30, m.m00*m.m23*m.m31 +
m.m01*m.m20*m.m33 + m.m03*m.m21*m.m30 - m.m00*m.m21*m.m33 - m.m01*m.m23*m.m30 - m.m03*m.m20*m.m31,
                                                m.m00*m.m11*m.m33 + m.m01*m.m13*m.m30 +
m.m03*m.m10*m.m31 - m.m00*m.m13*m.m31 - m.m01*m.m10*m.m33 - m.m03*m.m11*m.m30, m.m00*m.m13*m.m21 +
m.m01*m.m10*m.m23 + m.m03*m.m11*m.m20 - m.m00*m.m11*m.m23 - m.m01*m.m13*m.m20 - m.m03*m.m10*m.m21,

                                                m.m10*m.m22*m.m31 + m.m11*m.m20*m.m32 +
m.m12*m.m21*m.m30 - m.m10*m.m21*m.m32 - m.m11*m.m22*m.m30 - m.m12*m.m20*m.m31, m.m00*m.m21*m.m32 +
m.m01*m.m22*m.m30 + m.m02*m.m20*m.m31 - m.m00*m.m22*m.m31 - m.m01*m.m20*m.m32 - m.m02*m.m21*m.m30,
                                                m.m00*m.m12*m.m31 + m.m01*m.m10*m.m32 +
m.m02*m.m11*m.m30 - m.m00*m.m11*m.m32 - m.m01*m.m12*m.m30 - m.m02*m.m10*m.m31, m.m00*m.m11*m.m22 +
m.m01*m.m12*m.m20 + m.m02*m.m10*m.m21 - m.m00*m.m12*m.m21 - m.m01*m.m10*m.m22 - m.m02*m.m11*m.m20) /
det(m);
}

template<typename T>
Matrix4x4<T> affine_inverted(const Matrix4x4<T>& m) {
        float m00_11 = m.m00 * m.m11;
        float m01_12 = m.m01 * m.m12;
        float m02_10 = m.m02 * m.m10;
        float m00_12 = m.m00 * m.m12;
        float m01_10 = m.m01 * m.m10;
        float m02_11 = m.m02 * m.m11;

        float id = 1.f / (m00_11 * m.m22 + m01_12 * m.m20 + m02_10 * m.m21
                                        - m00_12 * m.m21 - m01_10 * m.m22 - m02_11 * m.m20);

        float m11_22 = m.m11 * m.m22;
        float m12_21 = m.m12 * m.m21;
        float m02_21 = m.m02 * m.m21;
        float m01_22 = m.m01 * m.m22;
        float m12_20 = m.m12 * m.m20;
        float m10_22 = m.m10 * m.m22;
        float m00_22 = m.m00 * m.m22;
        float m02_20 = m.m02 * m.m20;
        float m10_21 = m.m10 * m.m21;
        float m11_20 = m.m11 * m.m20;
        float m01_20 = m.m01 * m.m20;
        float m00_21 = m.m00 * m.m21;

        return Matrix4x4<T>(
                (m11_22 - m12_21) * id,
                (m02_21 - m01_22) * id,
                (m01_12 - m02_11) * id,
                0.f,

                (m12_20 - m10_22) * id,
                (m00_22 - m02_20) * id,
                (m02_10 - m00_12) * id,
                0.f,

                (m10_21 - m11_20) * id,
                (m01_20 - m00_21) * id,
                (m00_11 - m01_10) * id,
                0.f,

                (m10_22 * m.m31 + m11_20 * m.m32 + m12_21 * m.m30 - m10_21 * m.m32 - m11_22 * m.m30
- m12_20 * m.m31) * id, (m00_21 * m.m32 + m01_22 * m.m30 + m02_20 * m.m31 - m00_22 * m.m31 - m01_20
* m.m32 - m02_21 * m.m30) * id, (m00_12 * m.m31 + m01_10 * m.m32 + m02_11 * m.m30 - m00_11 * m.m32 -
m01_12 * m.m30 - m02_10 * m.m31) * id, 1.f);
}

template<typename T>
void compose(Matrix4x4<T>& m,
                                                        const Matrix3x3<T>& basis,
                                                        Vector3<T> const& scale,
                                                        Vector3<T> const& origin) {
        m.m00 = basis.m00 * scale.x; m.m01 = basis.m01 * scale.x; m.m02 = basis.m02 * scale.x; m.m03
= T(0); m.m10 = basis.m10 * scale.y; m.m11 = basis.m11 * scale.y; m.m12 = basis.m12 * scale.y; m.m13
= T(0); m.m20 = basis.m20 * scale.z; m.m21 = basis.m21 * scale.z; m.m22 = basis.m22 * scale.z; m.m23
= T(0); m.m30 = origin.x;			 m.m31 = origin.y;			  m.m32 =
origin.z;			   m.m33 = T(1);

}
*/
/****************************************************************************
 *
 * Aligned 4x4 float matrix
 *
 ****************************************************************************/

inline Matrix4x4f_a::Matrix4x4f_a() noexcept = default;

inline Matrix4x4f_a::Matrix4x4f_a(float m00, float m01, float m02, float m03, float m10, float m11,
                                  float m12, float m13, float m20, float m21, float m22, float m23,
                                  float m30, float m31, float m32, float m33) noexcept
    : r{{m00, m01, m02, m03}, {m10, m11, m12, m13}, {m20, m21, m22, m23}, {m30, m31, m32, m33}} {}

inline Matrix4x4f_a::Matrix4x4f_a(Matrix3x3f_a const& m) noexcept
    : r{Vector4f_a(m.r[0]), Vector4f_a(m.r[1]), Vector4f_a(m.r[2]), {0.f, 0.f, 0.f, 1.f}} {}

static inline Matrix4x4f_a compose(Matrix3x3f_a const& basis, Vector3f_a const& scale,
                                   Vector3f_a const& origin) noexcept {
    return Matrix4x4f_a(basis.r[0][0] * scale[0], basis.r[0][1] * scale[0],
                        basis.r[0][2] * scale[0], 0.f, basis.r[1][0] * scale[1],
                        basis.r[1][1] * scale[1], basis.r[1][2] * scale[1], 0.f,
                        basis.r[2][0] * scale[2], basis.r[2][1] * scale[2],
                        basis.r[2][2] * scale[2], 0.f, origin[0], origin[1], origin[2], 1.f);
}

static inline Matrix4x4f_a compose(Matrix4x4f_a const& basis, Vector3f_a const& scale,
                                   Vector3f_a const& origin) noexcept {
    return Matrix4x4f_a(basis.r[0][0] * scale[0], basis.r[0][1] * scale[0],
                        basis.r[0][2] * scale[0], 0.f, basis.r[1][0] * scale[1],
                        basis.r[1][1] * scale[1], basis.r[1][2] * scale[1], 0.f,
                        basis.r[2][0] * scale[2], basis.r[2][1] * scale[2],
                        basis.r[2][2] * scale[2], 0.f, origin[0], origin[1], origin[2], 1.f);
}

static inline void decompose(Matrix4x4f_a const& m, Matrix3x3f_a& basis, Vector3f_a& scale,
                             Vector3f_a& origin) {
    float3 const s(length(m.x()), length(m.y()), length(m.z()));

    basis  = Matrix3x3f_a(m.x() / s[0], m.y() / s[1], m.z() / s[2]);
    scale  = s;
    origin = m.w();
}

inline Matrix4x4f_a::Matrix4x4f_a(Transformation const& t) noexcept
    : Matrix4x4f_a(compose(quaternion::create_matrix3x3(t.rotation), t.scale, t.position)) {}

inline Vector3f_a Matrix4x4f_a::x() const noexcept {
    return r[0].xyz();
}

inline Vector3f_a Matrix4x4f_a::y() const noexcept {
    return r[1].xyz();
}

inline Vector3f_a Matrix4x4f_a::z() const noexcept {
    return r[2].xyz();
}

inline Vector3f_a Matrix4x4f_a::w() const noexcept {
    return r[3].xyz();
}

static inline Matrix4x4f_a operator*(Matrix4x4f_a const& a, Matrix4x4f_a const& b) noexcept {
    return Matrix4x4f_a((a.r[0][0] * b.r[0][0] + a.r[0][1] * b.r[1][0]) +
                            (a.r[0][2] * b.r[2][0] + a.r[0][3] * b.r[3][0]),
                        (a.r[0][0] * b.r[0][1] + a.r[0][1] * b.r[1][1]) +
                            (a.r[0][2] * b.r[2][1] + a.r[0][3] * b.r[3][1]),
                        (a.r[0][0] * b.r[0][2] + a.r[0][1] * b.r[1][2]) +
                            (a.r[0][2] * b.r[2][2] + a.r[0][3] * b.r[3][2]),
                        (a.r[0][0] * b.r[0][3] + a.r[0][1] * b.r[1][3]) +
                            (a.r[0][2] * b.r[2][3] + a.r[0][3] * b.r[3][3]),

                        (a.r[1][0] * b.r[0][0] + a.r[1][1] * b.r[1][0]) +
                            (a.r[1][2] * b.r[2][0] + a.r[1][3] * b.r[3][0]),
                        (a.r[1][0] * b.r[0][1] + a.r[1][1] * b.r[1][1]) +
                            (a.r[1][2] * b.r[2][1] + a.r[1][3] * b.r[3][1]),
                        (a.r[1][0] * b.r[0][2] + a.r[1][1] * b.r[1][2]) +
                            (a.r[1][2] * b.r[2][2] + a.r[1][3] * b.r[3][2]),
                        (a.r[1][0] * b.r[0][3] + a.r[1][1] * b.r[1][3]) +
                            (a.r[1][2] * b.r[2][3] + a.r[1][3] * b.r[3][3]),

                        (a.r[2][0] * b.r[0][0] + a.r[2][1] * b.r[1][0]) +
                            (a.r[2][2] * b.r[2][0] + a.r[2][3] * b.r[3][0]),
                        (a.r[2][0] * b.r[0][1] + a.r[2][1] * b.r[1][1]) +
                            (a.r[2][2] * b.r[2][1] + a.r[2][3] * b.r[3][1]),
                        (a.r[2][0] * b.r[0][2] + a.r[2][1] * b.r[1][2]) +
                            (a.r[2][2] * b.r[2][2] + a.r[2][3] * b.r[3][2]),
                        (a.r[2][0] * b.r[0][3] + a.r[2][1] * b.r[1][3]) +
                            (a.r[2][2] * b.r[2][3] + a.r[2][3] * b.r[3][3]),

                        (a.r[3][0] * b.r[0][0] + a.r[3][1] * b.r[1][0]) +
                            (a.r[3][2] * b.r[2][0] + a.r[3][3] * b.r[3][0]),
                        (a.r[3][0] * b.r[0][1] + a.r[3][1] * b.r[1][1]) +
                            (a.r[3][2] * b.r[2][1] + a.r[3][3] * b.r[3][1]),
                        (a.r[3][0] * b.r[0][2] + a.r[3][1] * b.r[1][2]) +
                            (a.r[3][2] * b.r[2][2] + a.r[3][3] * b.r[3][2]),
                        (a.r[3][0] * b.r[0][3] + a.r[3][1] * b.r[1][3]) +
                            (a.r[3][2] * b.r[2][3] + a.r[3][3] * b.r[3][3]));
}

static inline Vector3f_a transform_vector(Matrix4x4f_a const& m, Vector3f_a const& v) noexcept {
    return Vector3f_a(v[0] * m.r[0][0] + v[1] * m.r[1][0] + v[2] * m.r[2][0],
                      v[0] * m.r[0][1] + v[1] * m.r[1][1] + v[2] * m.r[2][1],
                      v[0] * m.r[0][2] + v[1] * m.r[1][2] + v[2] * m.r[2][2]);
}

static inline Vector3f_a transform_vector_transposed(Matrix4x4f_a const& m,
                                                     Vector3f_a const&   v) noexcept {
    return Vector3f_a(v[0] * m.r[0][0] + v[1] * m.r[0][1] + v[2] * m.r[0][2],
                      v[0] * m.r[1][0] + v[1] * m.r[1][1] + v[2] * m.r[1][2],
                      v[0] * m.r[2][0] + v[1] * m.r[2][1] + v[2] * m.r[2][2]);
}

static inline Vector3f_a transform_point(Matrix4x4f_a const& m, Vector3f_a const& v) noexcept {
    return Vector3f_a((v[0] * m.r[0][0] + v[1] * m.r[1][0]) + (v[2] * m.r[2][0] + m.r[3][0]),
                      (v[0] * m.r[0][1] + v[1] * m.r[1][1]) + (v[2] * m.r[2][1] + m.r[3][1]),
                      (v[0] * m.r[0][2] + v[1] * m.r[1][2]) + (v[2] * m.r[2][2] + m.r[3][2]));
}

static inline Matrix4x4f_a affine_inverted(Matrix4x4f_a const& m) noexcept {
    Matrix4x4f_a o;

    float id;

    {
        float const m00_11 = m.r[0][0] * m.r[1][1];
        float const m01_12 = m.r[0][1] * m.r[1][2];
        float const m02_10 = m.r[0][2] * m.r[1][0];
        float const m00_12 = m.r[0][0] * m.r[1][2];
        float const m01_10 = m.r[0][1] * m.r[1][0];
        float const m02_11 = m.r[0][2] * m.r[1][1];

        id = 1.f / ((m00_11 * m.r[2][2] + m01_12 * m.r[2][0] + m02_10 * m.r[2][1]) -
                    (m00_12 * m.r[2][1] + m01_10 * m.r[2][2] + m02_11 * m.r[2][0]));

        o.r[0][2] = (m01_12 - m02_11) * id;
        o.r[1][2] = (m02_10 - m00_12) * id;
        o.r[2][2] = (m00_11 - m01_10) * id;
        o.r[3][2] = ((m00_12 * m.r[3][1] + m01_10 * m.r[3][2] + m02_11 * m.r[3][0]) -
                     (m00_11 * m.r[3][2] + m01_12 * m.r[3][0] + m02_10 * m.r[3][1])) *
                    id;
    }

    {
        float const m11_22 = m.r[1][1] * m.r[2][2];
        float const m12_21 = m.r[1][2] * m.r[2][1];
        float const m12_20 = m.r[1][2] * m.r[2][0];
        float const m10_22 = m.r[1][0] * m.r[2][2];
        float const m10_21 = m.r[1][0] * m.r[2][1];
        float const m11_20 = m.r[1][1] * m.r[2][0];

        o.r[0][0] = (m11_22 - m12_21) * id;
        o.r[1][0] = (m12_20 - m10_22) * id;
        o.r[2][0] = (m10_21 - m11_20) * id;
        o.r[3][0] = ((m10_22 * m.r[3][1] + m11_20 * m.r[3][2] + m12_21 * m.r[3][0]) -
                     (m10_21 * m.r[3][2] + m11_22 * m.r[3][0] + m12_20 * m.r[3][1])) *
                    id;
    }

    {
        float const m02_21 = m.r[0][2] * m.r[2][1];
        float const m01_22 = m.r[0][1] * m.r[2][2];
        float const m00_22 = m.r[0][0] * m.r[2][2];
        float const m02_20 = m.r[0][2] * m.r[2][0];
        float const m01_20 = m.r[0][1] * m.r[2][0];
        float const m00_21 = m.r[0][0] * m.r[2][1];

        o.r[0][1] = (m02_21 - m01_22) * id;
        o.r[1][1] = (m00_22 - m02_20) * id;
        o.r[2][1] = (m01_20 - m00_21) * id;
        o.r[3][1] = ((m00_21 * m.r[3][2] + m01_22 * m.r[3][0] + m02_20 * m.r[3][1]) -
                     (m00_22 * m.r[3][1] + m01_20 * m.r[3][2] + m02_21 * m.r[3][0])) *
                    id;
    }

    {
        o.r[0][3] = 0.f;
        o.r[1][3] = 0.f;
        o.r[2][3] = 0.f;
        o.r[3][3] = 1.f;
    }

    return o;
}

static inline void set_translation(Matrix4x4f_a& m, Vector3f_a const& v) noexcept {
    m.r[0][0] = 1.f;
    m.r[0][1] = 0.f;
    m.r[0][2] = 0.f;
    m.r[0][3] = 0.f;
    m.r[1][0] = 0.f;
    m.r[1][1] = 1.f;
    m.r[1][2] = 0.f;
    m.r[1][3] = 0.f;
    m.r[2][0] = 0.f;
    m.r[2][1] = 0.f;
    m.r[2][2] = 1.f;
    m.r[2][3] = 0.f;
    m.r[3][0] = v[0];
    m.r[3][1] = v[1];
    m.r[3][2] = v[2];
    m.r[3][3] = 1.f;
}

static inline void set_rotation_x(Matrix4x4f_a& m, float a) noexcept {
    float const c = std::cos(a);
    float const s = std::sin(a);

    m.r[0][0] = 1.f;
    m.r[0][1] = 0.f;
    m.r[0][2] = 0.f;
    m.r[0][3] = 0.f;
    m.r[1][0] = 0.f;
    m.r[1][1] = c;
    m.r[1][2] = -s;
    m.r[1][3] = 0.f;
    m.r[2][0] = 0.f;
    m.r[2][1] = s;
    m.r[2][2] = c;
    m.r[2][3] = 0.f;
    m.r[3][0] = 0.f;
    m.r[3][1] = 0.f;
    m.r[3][2] = 0.f;
    m.r[3][3] = 1.f;
}

static inline void set_rotation_y(Matrix4x4f_a& m, float a) noexcept {
    float const c = std::cos(a);
    float const s = std::sin(a);

    m.r[0][0] = c;
    m.r[0][1] = 0.f;
    m.r[0][2] = s;
    m.r[0][3] = 0.f;
    m.r[1][0] = 0.f;
    m.r[1][1] = 1.f;
    m.r[1][2] = 0.f;
    m.r[1][3] = 0.f;
    m.r[2][0] = -s;
    m.r[2][1] = 0.f;
    m.r[2][2] = c;
    m.r[2][3] = 0.f;
    m.r[3][0] = 0.f;
    m.r[3][1] = 0.f;
    m.r[3][2] = 0.f;
    m.r[3][3] = 1.f;
}

static inline void set_view(Matrix4x4f_a& m, Matrix3x3f_a const& basis,
                            Vector3f_a const& eye) noexcept {
    m.r[0][0] = basis.r[0][0];
    m.r[0][1] = basis.r[1][0];
    m.r[0][2] = basis.r[2][0];
    m.r[0][3] = 0.f;

    m.r[1][0] = basis.r[0][1];
    m.r[1][1] = basis.r[1][1];
    m.r[1][2] = basis.r[2][1];
    m.r[1][3] = 0.f;

    m.r[2][0] = basis.r[0][2];
    m.r[2][1] = basis.r[1][2];
    m.r[2][2] = basis.r[2][2];
    m.r[2][3] = 0.f;

    m.r[3][0] = -dot(basis.r[0], eye);
    m.r[3][1] = -dot(basis.r[1], eye);
    m.r[3][2] = -dot(basis.r[2], eye);
    m.r[3][3] = 1.f;
}

static inline void set_perspective(Matrix4x4f_a& m, float fov, float ratio) noexcept {
    /*
    xScale     0          0               0
    0        yScale       0               0
    0          0       zf/(zf-zn)         1
    0          0       -zn*zf/(zf-zn)     0
    where:
    yScale = cot(fovY/2)
    xScale = aspect ratio * yScale
    */

    float const t = fov * 0.5f;
    float const y = std::cos(t) / std::sin(t);
    float const x = y / ratio;

    m.r[0][0] = x;
    m.r[0][1] = 0.f;
    m.r[0][2] = 0.f;
    m.r[0][3] = 0.f;

    m.r[1][0] = 0.f;
    m.r[1][1] = y;
    m.r[1][2] = 0.f;
    m.r[1][3] = 0.f;

    m.r[2][0] = 0.f;
    m.r[2][1] = 0.f;
    m.r[2][2] = 1.f;
    m.r[2][3] = 1.f;

    m.r[3][0] = 0.f;
    m.r[3][1] = 0.f;
    m.r[3][2] = 0.f;
    m.r[3][3] = 0.f;
}

}  // namespace math

#endif
