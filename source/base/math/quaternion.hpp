#pragma once

#include "math.hpp"
#include "vector4.hpp"
#include "memory/const.hpp"

namespace math {

template<typename T> struct Vector3;
template<typename T> struct Matrix3x3;
struct Matrix3x3f_a;
struct Vector4f_a;

/****************************************************************************
 *
 * Generic quaternion
 *
 ****************************************************************************/

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
void set_rotation(Quaternion<T>& q, const Vector3<T>& v, T a);

template<typename T>
void set_rotation(Quaternion<T>& q, T yaw, T pitch, T roll);

template<typename T>
Quaternion<T> slerp(const Quaternion<T>& a, const Quaternion<T>& b, T t);

/****************************************************************************
 *
 * Aligned quaternon functions
 *
 ****************************************************************************/

namespace quat {

// using quaternion = Quaternion<float>;
using quaternion = Vector4f_a;

quaternion create(const Matrix3x3<float>& m);

quaternion create(const Matrix3x3f_a& m);

quaternion create_rotation_x(float a);

quaternion create_rotation_y(float a);

quaternion create_rotation_z(float a);

quaternion mul(const quaternion& a, const quaternion& b);

quaternion slerp(const quaternion& a, const quaternion& b, float t);

SU_GLOBALCONST(quaternion) identity(0.f, 0.f, 0.f, 1.f);

}

}

using quaternion = math::quat::quaternion;
