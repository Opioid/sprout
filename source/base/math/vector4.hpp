#pragma once

#include "vector3.hpp"
#include "memory/const.hpp"

namespace math {

struct Vector4f_a;

using FVector4f_a = const Vector4f_a&;

/****************************************************************************
 *
 * Generic 4D vector
 *
 ****************************************************************************/

template<typename T>
struct Vector4 {
	T v[4];

	Vector4();

	Vector4(T x, T y, T z, T w = T(1));

	explicit Vector4(T s);

	explicit Vector4(Vector2<T> xy, T z, T w = T(1));

	explicit Vector4(const Vector3<T>& xyz, T w = T(1));

	Vector4 operator+(const Vector4& v) const;

	Vector4 operator*(const Vector4& v) const;

	Vector4 operator/(T s) const;

	Vector4& operator+=(const Vector4& v);

	Vector4& operator-=(const Vector4& v);

	static const Vector4 identity;
};

template<typename T>
Vector4<T> operator*(T s, const Vector4<T> &v);

template<typename T>
T dot(const Vector4<T>& a, const Vector4<T>& b);

/****************************************************************************
 *
 * Aligned 4D float vector
 *
 ****************************************************************************/

struct alignas(16) Vector4f_a {
	float v[4];

	Vector4f_a();

	Vector4f_a(float x, float y, float z, float w = 1.f);

	explicit Vector4f_a(float s);

	explicit Vector4f_a(Vector2<float> xy, float z, float w = 1.f);

	explicit Vector4f_a(FVector3f_a xyz, float w = 1.f);

	explicit Vector4f_a(const Vector3<float>& xyz, float w = 1.f);

	Vector3f_a xyz() const;

	Vector4f_a operator+(const Vector4f_a& v) const;

	Vector4f_a operator*(const Vector4f_a& v) const;

	Vector4f_a operator/(float s) const;

	Vector4f_a& operator+=(const Vector4f_a& v);

	Vector4f_a& operator-=(const Vector4f_a& v);

	bool operator==(FVector4f_a v) const;

	bool operator!=(FVector4f_a v) const;
};

Vector4f_a operator*(float s, const Vector4f_a& v);

float dot(const Vector4f_a& a, const Vector4f_a& b);

SU_GLOBALCONST(Vector4f_a) float4_identity(0.f);

/****************************************************************************
 *
 * Aligned 4D int vector
 *
 ****************************************************************************/

struct alignas(16) Vector4i_a {
	int32_t v[4];

	Vector4i_a();

	Vector4i_a(int32_t x, int32_t y, int32_t z, int32_t w);

	Vector4i_a(Vector2<int32_t> xy, Vector2<int32_t> zw);

	explicit Vector4i_a(int32_t s);

	const Vector2<int32_t> xy() const;
	const Vector2<int32_t> zw() const;

	Vector4i_a operator+(const Vector4i_a& v) const;

	Vector4i_a operator*(const Vector4i_a& v) const;

	Vector4i_a& operator+=(const Vector4i_a& v);

	Vector4i_a& operator-=(const Vector4i_a& v);
};

}
