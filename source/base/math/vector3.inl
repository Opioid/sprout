#pragma once

#include "vector3.hpp"
#include <cmath>
#include <algorithm>

namespace math {

template<typename T>
inline Vector3<T>::Vector3() {}

template<typename T>
inline Vector3<T>::Vector3(T x, T y, T z) : x(x), y(y), z(z)  {}

template<typename T>
inline Vector3<T>::Vector3(const Vector2<T>& xy, T z) : xy(xy), _z(z) {}

template<typename T>
inline Vector3<T>::Vector3(const T* v) : x(v[0]), y(v[1]), z(v[2]) {}

template<typename T>
inline Vector3<T> Vector3<T>::operator+(T s) const {
	return Vector3(x + s, y + s, z + s);
}

template<typename T>
inline Vector3<T> Vector3<T>::operator+(const Vector3& v) const {
	return Vector3(x + v.x, y + v.y, z + v.z);
}

template<typename T>
inline Vector3<T> Vector3<T>::operator-(T s) const {
	return Vector3(x - s, y - s, z - s);
}

template<typename T>
inline Vector3<T> Vector3<T>::operator-(const Vector3& v) const {
	return Vector3(x - v.x, y - v.y, z - v.z);
}

template<typename T>
inline Vector3<T> Vector3<T>::operator*(const Vector3& v) const {
	return Vector3(x * v.x, y * v.y, z * v.z);
}

template<typename T>
inline Vector3<T> Vector3<T>::operator/(T s) const {
	T is = T(1) / s;
	return Vector3(is * x, is * y, is * z);
}

template<typename T>
inline Vector3<T> Vector3<T>::operator/(const Vector3& v) const {
	return Vector3(x / v.x, y / v.y, z / v.z);
}

template<typename T>
inline Vector3<T> Vector3<T>::operator-() const {
	return Vector3(-x, -y, -z);
}

template<typename T>
inline Vector3<T>& Vector3<T>::operator+=(const Vector3& v) {
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

template<typename T>
inline Vector3<T>& Vector3<T>::operator-=(const Vector3& v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

template<typename T>
inline Vector3<T>& Vector3<T>::operator*=(const Vector3& v) {
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

template<typename T>
inline Vector3<T>& Vector3<T>::operator*=(T s) {
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

template<typename T>
inline Vector3<T>& Vector3<T>::operator/=(T s) {
	T is = T(1) / s;
	x *= is;
	y *= is;
	z *= is;
	return *this;
}

template<typename T>
inline bool Vector3<T>::operator==(const Vector3& v) const {
	return x == v.x && y == v.y && z == v.z;
}

template<typename T>
inline bool Vector3<T>::operator!=(const Vector3& v) const {
	return x != v.x || y != v.y || z != v.z;
}

template<typename T>
inline Vector3<T>::operator unsigned int() const {
/*	const __m128 m4x255f = _mm_set_ps1(255.f);

	__m128 m0 = _mm_set_ps(b, g, r, 0.f);

	m0 = _mm_mul_ps(m0, m4x255f);

	__m128i m1 = _mm_cvtps_epi32(m0);

	if (m1.m128i_i32[3] > 255) m1.m128i_i32[3] = 255;
	if (m1.m128i_i32[2] > 255) m1.m128i_i32[2] = 255;
	if (m1.m128i_i32[1] > 255) m1.m128i_i32[1] = 255;

	return 0xff000000 | (m1.m128i_i32[3] << 16) | (m1.m128i_i32[2] << 8) | m1.m128i_i32[1];
	*/

	unsigned int red  (x * T(255));
	unsigned int green(y * T(255));
	unsigned int blue (z * T(255));

	return 0xff000000 | (blue << 16) | (green << 8) | red;
}

template<typename T>
inline T Vector3<T>::absolute_max(uint32_t& i) const {
	T ax = std::abs(x);
	T ay = std::abs(y);
	T az = std::abs(z);

	if (ax >= ay && ax >= az) {
		i = 0;
		return ax;
	}

	if (ay >= ax && ay >= az) {
		i = 1;
		return ay;
	}

	i = 2;
	return az;
}

template<typename T>
const Vector3<T> Vector3<T>::identity(T(0), T(0), T(0));

template<typename T>
inline Vector3<T> operator*(T s, const Vector3<T>& v) {
	return Vector3<T>(s * v.x, s * v.y, s * v.z);
}

template<typename T>
inline T dot(const Vector3<T>& a, const Vector3<T>& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<typename T>
inline T length(const Vector3<T>& v) {
	return std::sqrt(dot(v, v));
}

template<typename T>
inline T squared_length(const Vector3<T>& v) {
	return dot(v, v);
}

template<typename T>
inline Vector3<T> normalized(const Vector3<T>& v) {
	return v / length(v);
}

template<typename T>
inline Vector3<T> reciprocal(const Vector3<T>& v) {
	return Vector3<T>(T(1) / v.x, T(1) / v.y, T(1) / v.z);
}

template<typename T>
inline Vector3<T> cross(const Vector3<T>& a, const Vector3<T>& b) {
	return Vector3<T>(a.y * b.z - a.z * b.y,
					  a.z * b.x - a.x * b.z,
					  a.x * b.y - a.y * b.x);
}

template<typename T>
inline Vector3<T> project(const Vector3<T>& a, const Vector3<T>& b) {
	return dot(b, a) * b;
}

template<typename T>
inline T distance(const Vector3<T>& a, const Vector3<T>& b) {
	return length(a - b);
}

template<typename T>
inline T squared_distance(const Vector3<T>& a, const Vector3<T>& b) {
	return squared_length(a - b);
}

template<typename T>
inline Vector3<T> saturate(const Vector3<T>& v) {
	return Vector3<T>(std::min(std::max(v.x, T(0)), T(1)), std::min(std::max(v.y, T(0)), T(1)), std::min(std::max(v.z, T(0)), T(1)));
}

template<typename T>
inline Vector3<T> lerp(const Vector3<T>& a, const Vector3<T>& b, T t) {
	T u = T(1) - t;
	return u * a + t * b;
}

template<typename T>
inline Vector3<T> reflect(const Vector3<T>& normal, const Vector3<T>& v) {
	return v - T(2) * dot(v, normal) * normal;
}

template<typename T>
void coordinate_system(const Vector3<T>& n, Vector3<T>& t, Vector3<T>& b) {
	Vector3<T> r1;

	if (n.x < T(0.6) && n.x > T(-0.6)) {
		r1 = Vector3<T>(T(1), T(0), T(0));
	} else if (n.y < T(0.6) && n.y > T(-0.6)) {
		r1 = Vector3<T>(T(0), T(1), T(0));
	} else {
		r1 = Vector3<T>(T(0), T(0), T(1));
	}

	Vector3<T> r0 = normalized(cross(n, r1));

	t = r0;
	b = cross(r0, n);
}

template<typename T>
inline Vector3<T> min(const Vector3<T>& a, const Vector3<T>& b) {
	return Vector3<T>(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

template<typename T>
inline Vector3<T> max(const Vector3<T>& a, const Vector3<T>& b) {
	return Vector3<T>(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}

template<typename T>
inline Vector3<T> abs(const Vector3<T>& v) {
	return Vector3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

template<typename T>
bool contains_nan(const Vector3<T>& v) {
	return std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z);
}

template<typename T>
bool contains_inf(const Vector3<T>& v) {
	return std::isinf(v.x) || std::isinf(v.y) || std::isinf(v.z);
}

}
