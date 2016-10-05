#pragma once

#include "vector4.hpp"

namespace math {

/****************************************************************************
 *
 * Generic 4D vector
 *
 ****************************************************************************/

template<typename T>
Vector4<T>::Vector4() {}

template<typename T>
Vector4<T>::Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

template<typename T>
Vector4<T>::Vector4(T s) : x(s), y(s), z(s), w(s) {}

template<typename T>
Vector4<T>::Vector4(Vector2<T> xy, T z, T w) : x(xy.x), y(xy.y), z(z), w(w) {}

template<typename T>
Vector4<T>::Vector4(const Vector3<T>& xyz, T w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}

template<typename T>
Vector4<T> Vector4<T>::operator+(const Vector4& v) const {
	return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
}

template<typename T>
Vector4<T> Vector4<T>::operator*(const Vector4& v) const {
	return Vector4(x * v.x, y * v.y, z * v.z, w * v.w);
}

template<typename T>
Vector4<T> Vector4<T>::operator/(T s) const {
	T is = T(1) / s;
	return Vector4(is * x, is * y, is * z, is * w);
}

template<typename T>
Vector4<T>& Vector4<T>::operator+=(const Vector4& v) {
	x += v.x; y += v.y; z += v.z; w += v.w;
	return *this;
}

template<typename T>
Vector4<T>& Vector4<T>::operator-=(const Vector4& v) {
	x -= v.x; y -= v.y; z -= v.z; w -= v.w;
	return *this;
}

template<typename T>
const Vector4<T> Vector4<T>::identity(T(0), T(0), T(0), T(0));

template<typename T>
Vector4<T> operator*(T s, const Vector4<T> &v) {
	return Vector4<T>(s * v.x, s * v.y, s * v.z, s * v.w);
}

template<typename T>
T dot(const Vector4<T>& a, const Vector4<T>& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

/****************************************************************************
 *
 * Aligned 4D float vector
 *
 ****************************************************************************/

inline Vector4f_a::Vector4f_a() {}

inline Vector4f_a::Vector4f_a(float x, float y, float z, float w) :
	x(x), y(y), z(z), w(w) {}

inline Vector4f_a::Vector4f_a(float s) :
	x(s), y(s), z(s), w(s) {}

inline Vector4f_a::Vector4f_a(Vector2<float> xy, float z, float w) :
	x(xy.x), y(xy.y), z(z), w(w) {}

inline Vector4f_a::Vector4f_a(const Vector3f_a& xyz, float w) :
	x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}

inline Vector4f_a::Vector4f_a(const Vector3<float>& xyz, float w) :
	x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}

inline Vector4f_a Vector4f_a::operator+(const Vector4f_a& v) const {
	return Vector4f_a(x + v.x, y + v.y, z + v.z, w + v.w);
}

inline Vector4f_a Vector4f_a::operator*(const Vector4f_a& v) const {
	return Vector4f_a(x * v.x, y * v.y, z * v.z, w * v.w);
}

inline Vector4f_a Vector4f_a::operator/(float s) const {
	float is = 1.f / s;
	return Vector4f_a(is * x, is * y, is * z, is * w);
}

inline Vector4f_a& Vector4f_a::operator+=(const Vector4f_a& v) {
	x += v.x; y += v.y; z += v.z; w += v.w;
	return *this;
}

inline Vector4f_a& Vector4f_a::operator-=(const Vector4f_a& v) {
	x -= v.x; y -= v.y; z -= v.z; w -= v.w;
	return *this;
}

inline Vector4f_a operator*(float s, const Vector4f_a& v) {
	return Vector4f_a(s * v.x, s * v.y, s * v.z, s * v.w);
}

inline float dot(const Vector4f_a& a, const Vector4f_a& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

}
