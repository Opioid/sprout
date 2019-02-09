#ifndef SU_BASE_MATH_MATRIX3X3_HPP
#define SU_BASE_MATH_MATRIX3X3_HPP

#include "matrix.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

namespace math {

template <typename T>
struct Matrix4x4;

/****************************************************************************
 *
 * Generic 3x3 matrix
 *
 ****************************************************************************/
/*
template<typename T>
struct Matrix3x3 {
        Vector3<T> r[3];

        Matrix3x3();

        Matrix3x3(T m00, T m01, T m02,
                          T m10, T m11, T m12,
                          T m20, T m21, T m22);
        Matrix3x3(Vector3<T> const& x, Vector3<T> const& y, Vector3<T> const& z);
        explicit Matrix3x3(const Matrix4x4<T>& m);

        Matrix3x3 operator*(const Matrix3x3& m) const;

        Matrix3x3 operator/(T s) const;

        Matrix3x3& operator*=(const Matrix3x3& m);

        static Matrix3x3 identity();
};

template<typename T>
Vector3<T> operator*(Vector3<T> const& v, const Matrix3x3<T>& m);

template<typename T>
Vector3<T>& operator*=(Vector3<T>& v, const Matrix3x3<T>& m);

template<typename T>
Vector3<T> transform_vector(const Matrix3x3<T>& m, Vector3<T> const& v);

template<typename T>
Vector3<T> transform_vector_transposed(const Matrix3x3<T>& m, Vector3<T> const& v);

template<typename T>
void transform_vectors(const Matrix3x3<T>& m,
                                           Vector3<T> const& a, Vector3<T> const& b, Vector3<T>& oa,
Vector3<T>& ob);

template<typename T>
Matrix3x3<T> normalize(const Matrix3x3<T>& m);

template<typename T>
T det(const Matrix3x3<T>& m);

template<typename T>
Matrix3x3<T> invert(const Matrix3x3<T>& m);

template<typename T>
void set_look_at(Matrix3x3<T>& m, Vector3<T> const& dir, Vector3<T> const& up);

// template<typename T>
// void set_basis(Matrix3x3<T>& m, Vector3<T> const& v);

template<typename T>
void set_scale(Matrix3x3<T>& m, T x, T y, T z);

template<typename T>
void set_scale(Matrix3x3<T>& m, Vector3<T> const& v);

template<typename T>
void scale(Matrix3x3<T>& m, Vector3<T> const& v);

template<typename T>
void set_rotation_x(Matrix3x3<T>& m, T a);

template<typename T>
void set_rotation_y(Matrix3x3<T>& m, T a);

template<typename T>
void set_rotation_z(Matrix3x3<T>& m, T a);

template<typename T>
void set_rotation(Matrix3x3<T>& m, Vector3<T> const& v, T a);

template<typename T>
Matrix3x3<T> transposed(const Matrix3x3<T>& m);
*/
/****************************************************************************
 *
 * Aligned 3x3 float matrix
 *
 ****************************************************************************/

struct Vector4f_a;

struct alignas(16) Matrix3x3f_a {
    Vector3f_a r[3];

    Matrix3x3f_a() noexcept;

    constexpr Matrix3x3f_a(float m00, float m01, float m02, float m10, float m11, float m12,
                           float m20, float m21, float m22) noexcept;

    explicit constexpr Matrix3x3f_a(Vector3f_a const& x, Vector3f_a const& y,
                                    Vector3f_a const& z) noexcept;

    static Matrix3x3f_a constexpr identity() noexcept;
};

}  // namespace math

#endif
