#pragma once

#include "vector2.hpp"
#include "memory/align.hpp"
#include <cstdint>

namespace math {

template<typename T>
struct Vector3 {
	union {
		struct {
			T x, y, z;
		};

		T v[3];
	};

	Vector3();

	Vector3(T x, T y, T z);

	explicit Vector3(T s);

	explicit Vector3(Vector2<T> xy, T z = T(0));

	explicit Vector3(const T* v);

	Vector2<T> xy() const;

	Vector3 operator+(T s) const;

	Vector3 operator+(const Vector3& v) const;

	Vector3 operator-(T s) const;

	Vector3 operator-(const Vector3& v) const;
		
	Vector3 operator*(const Vector3& v) const;
		
	Vector3 operator/(T s) const;

	Vector3 operator/(const Vector3& v) const;

	Vector3 operator-() const;

	Vector3& operator+=(const Vector3& v);

	Vector3& operator-=(const Vector3& v);

	Vector3& operator*=(const Vector3& v);

	Vector3& operator*=(T s);

	Vector3& operator/=(T s);

	bool operator==(const Vector3& v) const;

	bool operator!=(const Vector3& v) const;

	explicit operator unsigned int() const;

	T absolute_max(uint32_t& i) const;

	static const Vector3 identity;
};

template<typename T>
Vector3<T> operator*(T s, const Vector3<T>& v);

template<typename T>
T dot(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
T length(const Vector3<T>& v);

template<typename T>
T squared_length(const Vector3<T>& v);

template<typename T>
Vector3<T> normalized(const Vector3<T>& v);

template<typename T>
Vector3<T> reciprocal(const Vector3<T>& v);

template<typename T>
Vector3<T> cross(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
Vector3<T> project(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
T distance(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
T squared_distance(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
Vector3<T> saturate(const Vector3<T>& v);

template<typename T>
Vector3<T> exp(const Vector3<T>& v);

template<typename T>
Vector3<T> lerp(const Vector3<T>& a, const Vector3<T>& b, T t);

template<typename T>
Vector3<T> reflect(const Vector3<T>& normal, const Vector3<T>& v);

// Assuming n is unit length
template<typename T>
void coordinate_system(const Vector3<T>& n, Vector3<T>& t, Vector3<T>& b);

template<typename T>
Vector3<T> min(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
Vector3<T> max(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
Vector3<T> abs(const Vector3<T>& v);

template<typename T>
bool contains_negative(const Vector3<T>& v);

template<typename T>
bool contains_greater_one(const Vector3<T>& v);

template<typename T>
bool contains_nan(const Vector3<T>& v);

template<typename T>
bool contains_inf(const Vector3<T>& v);

/****************************************************************************
 *
 * Aligned 3D float vector
 *
 ****************************************************************************/

struct ALIGN(16) Vector3fa {
	union {
		struct {
			float x, y, z;
		};

		float v[3];
	};

	Vector3fa();

	Vector3fa(float x, float y, float z);

	explicit Vector3fa(float s);

	explicit Vector3fa(const Vector3<float>& v);
};

}
