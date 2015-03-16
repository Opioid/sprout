#pragma once

#include "vector2.hpp"
#include <cstdint>

namespace math {

template<typename T>
struct Vector3 {
	union {
		struct {
			T x, y, z;
		};

		struct {
			Vector2<T> xy;
			T _z;
		};

		T v[3];
	};

	Vector3();

	Vector3(T x, T y, T z);

	explicit Vector3(const Vector2<T>& xy, T z = T(0));

	explicit Vector3(const T* v);

	Vector3 operator+(const Vector3& v) const;

	Vector3 operator-(const Vector3& v) const;
		
	Vector3 operator*(const Vector3& v) const;
		
	Vector3 operator/(T s) const;

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
Vector3<T> absolute(const Vector3<T>& v);

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
Vector3<T> lerp(const Vector3<T>& a, const Vector3<T>& b, T t);

template<typename T>
Vector3<T> reflect(const Vector3<T>& v, const Vector3<T>& normal);

template<typename T>
void coordinate_system(const Vector3<T>& n, Vector3<T>& t, Vector3<T>& b);

Vector3<float> min(const Vector3<float>& a, const Vector3<float>& b);

Vector3<float> max(const Vector3<float>& a, const Vector3<float>& b);

}
