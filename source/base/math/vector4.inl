#pragma once

#include "vector4.hpp"

namespace math {

template<typename T>
Vector4<T>::Vector4() {}

template<typename T>
Vector4<T>::Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

template<typename T>
Vector4<T>::Vector4(Vector2<T> xy, T z, T w) : xy(xy), zw(z, w) {}

template<typename T>
Vector4<T>::Vector4(Vector2<T> xy, Vector2<T> zw) : xy(xy), zw(zw) {}

template<typename T>
Vector4<T>::Vector4(const Vector3<T>& xyz, T w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}

template<typename T>
Vector3<T> Vector4<T>::xyz() const {
	return Vector3<T>(x, y, z);
}

template<typename T>
Vector4<T> Vector4<T>::operator+(const Vector4& v) const {
	return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
}

template<typename T>
Vector4<T> Vector4<T>::operator*(const Vector4& v) const {
	return Vector4(x * v.x, y * v.y, z * v.z, w * v.w);
}

template<typename T>
Vector4<T> Vector4<T>::operator*(const Vector3<T>& v) const {
	return Vector4(x * v.x, y * v.y, z * v.z, w);
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
Vector4<T>::operator uint32_t() const {
	/*
	const __m128 m4x255f = _mm_set_ps1(255.f);

	__m128 m0 = _mm_set_ps(r, g, b, a);

	m0 = _mm_mul_ps(m0, m4x255f);

	__m128i m1 = _mm_cvtps_epi32(m0);

	if (m1.m128i_i32[3] > 255)
	{
		m1.m128i_i32[3] = 255;
	}

	if (m1.m128i_i32[2] > 255)
	{
		m1.m128i_i32[2] = 255;
	}

	if (m1.m128i_i32[1] > 255)
	{
		m1.m128i_i32[1] = 255;
	}

	if (m1.m128i_i32[0] > 255)
	{
		m1.m128i_i32[0] = 255;
	}

	return  (m1.m128i_i32[0] << 24) | (m1.m128i_i32[1] << 16) | (m1.m128i_i32[2] << 8) | m1.m128i_i32[3];
	*/

	uint32_t red   = static_cast<uint32_t>(x * T(255));
	uint32_t green = static_cast<uint32_t>(y * T(255));
	uint32_t blue  = static_cast<uint32_t>(z * T(255));
	uint32_t alpha = static_cast<uint32_t>(w * T(255));

	return  (alpha << 24) | (blue << 16) | (green << 8) | red;
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

}
