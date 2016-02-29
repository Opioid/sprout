#pragma once

namespace math {

template<typename T> struct Vector2;
template<typename T> struct Vector3;
struct alignas(16) Vector3f_a;

/****************************************************************************
 *
 * Generic 4D vector
 *
 ****************************************************************************/

template<typename T>
struct Vector4
{
	union
	{
		struct
		{
			T x, y, z, w;
		};

		Vector3<T> xyz;

		T v[4];
	};

	Vector4();

	Vector4(T x, T y, T z, T w = T(1));

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

struct alignas(16) Vector4f_a
{
	union
	{
		struct
		{
			float x, y, z, w;
		};

		Vector3f_a xyz;

		float v[4];
	};

	Vector4f_a();

	Vector4f_a(float x, float y, float z, float w = 1.f);

	explicit Vector4f_a(Vector2<float> xy, float z, float w = 1.f);

	explicit Vector4f_a(const Vector3f_a& xyz, float w = 1.f);

	explicit Vector4f_a(const Vector3<float>& xyz, float w = 1.f);

	Vector4f_a operator+(const Vector4f_a& v) const;

	Vector4f_a operator*(const Vector4f_a& v) const;

	Vector4f_a operator/(float s) const;

	Vector4f_a& operator+=(const Vector4f_a& v);

	Vector4f_a& operator-=(const Vector4f_a& v);
};

Vector4f_a operator*(float s, const Vector4f_a& v);

float dot(const Vector4f_a& a, const Vector4f_a& b);

}
