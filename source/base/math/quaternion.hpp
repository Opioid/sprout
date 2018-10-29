#ifndef SU_BASE_MATH_QUATERNION_HPP
#define SU_BASE_MATH_QUATERNION_HPP

#include "vector4.hpp"

namespace math {

template <typename T>
struct Vector3;
template <typename T>
struct Matrix3x3;
struct Matrix3x3f_a;

/****************************************************************************
 *
 * Generic quaternion
 *
 ****************************************************************************/
/*
template<typename T>
struct Quaternion {

        union {
                struct {
                        T x, y, z, w;
                };

                T v[4];
        };

        Quaternion();

        Quaternion(T x, T y, T z, T w);

        explicit Quaternion(const Matrix3x3<T>& m);

        Quaternion operator*(const Quaternion& q) const;

        static const Quaternion identity;
};

template<typename T>
T dot(const Quaternion<T>& a, const Quaternion<T>& b);

template<typename T>
T length(const Quaternion<T>& q);

template<typename T>
T angle(const Quaternion<T>& a, const Quaternion<T>& b);

template<typename T>
void set_rotation_x(Quaternion<T>& q, T a);

template<typename T>
void set_rotation_y(Quaternion<T>& q, T a);

template<typename T>
void set_rotation_z(Quaternion<T>& q, T a);

template<typename T>
void set_rotation(Quaternion<T>& q, Vector3<T> const& v, T a);

template<typename T>
void set_rotation(Quaternion<T>& q, T yaw, T pitch, T roll);

template<typename T>
Quaternion<T> slerp(const Quaternion<T>& a, const Quaternion<T>& b, T t);
*/
/****************************************************************************
 *
 * Aligned quaternon functions
 *
 ****************************************************************************/

// using quaternion = Quaternion<float>;
using Quaternion = float4;

}  // namespace math

#endif
