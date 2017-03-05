#pragma once

#include "vector3.hpp"
#include "simd/simd_math.inl"
#include "simd/simd_vector.inl"
#include <cmath>
#include <algorithm>

namespace math {

/****************************************************************************
 *
 * Generic 3D vector
 *
 ****************************************************************************/

template<typename T>
Vector3<T>::Vector3() {}

template<typename T>
Vector3<T>::Vector3(T x, T y, T z) : x(x), y(y), z(z)  {}

template<typename T>
Vector3<T>::Vector3(T s) : x(s), y(s), z(s)  {}

template<typename T>
Vector3<T>::Vector3(Vector2<T> xy, T z) : x(xy.v[0]), y(xy.v[1]), z(z) {}

template<typename T>
Vector3<T>::Vector3(const T* v) : x(v[0]), y(v[1]), z(v[2]) {}

template<typename T>
Vector3<T>::Vector3(FVector3f_a v) : x(v.v[0]), y(v.v[1]), z(v.v[2]) {}

template<typename T>
Vector3<T> Vector3<T>::operator+(T s) const {
	return Vector3(x + s, y + s, z + s);
}

template<typename T>
Vector3<T> Vector3<T>::operator+(const Vector3& a) const {
	return Vector3(x + a.x, y + a.y, z + a.z);
}

template<typename T>
Vector3<T> Vector3<T>::operator-(T s) const {
	return Vector3(x - s, y - s, z - s);
}

template<typename T>
Vector3<T> Vector3<T>::operator-(const Vector3& a) const {
	return Vector3(x - a.x, y - a.y, z - a.z);
}

template<typename T>
Vector3<T> Vector3<T>::operator*(const Vector3& a) const {
	return Vector3(x * a.x, y * a.y, z * a.z);
}

template<typename T>
Vector3<T> Vector3<T>::operator/(T s) const {
	T is = T(1) / s;
	return Vector3(is * x, is * y, is * z);
}

template<typename T>
Vector3<T> Vector3<T>::operator/(const Vector3& a) const {
	return Vector3(x / a.x, y / a.y, z / a.z);
}

template<typename T>
Vector3<T> Vector3<T>::operator-() const {
	return Vector3(-x, -y, -z);
}

template<typename T>
Vector3<T>& Vector3<T>::operator+=(const Vector3& a) {
	x += a.x;
	y += a.y;
	z += a.z;
	return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator-=(const Vector3& a) {
	x -= a.x;
	y -= a.y;
	z -= a.z;
	return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator*=(const Vector3& a) {
	x *= a.x;
	y *= a.y;
	z *= a.z;
	return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator*=(T s) {
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator/=(T s) {
	T is = T(1) / s;
	x *= is;
	y *= is;
	z *= is;
	return *this;
}

template<typename T>
bool Vector3<T>::operator==(const Vector3& a) const {
	return x == a.x && y == a.y && z == a.z;
}

template<typename T>
bool Vector3<T>::operator!=(const Vector3& a) const {
	return x != a.x || y != a.y || z != a.z;
}

/*
template<typename T>
Vector3<T>::operator unsigned int() const {
	const __m128 m4x255f = _mm_set_ps1(255.f);

	__m128 m0 = _mm_set_ps(b, g, r, 0.f);

	m0 = _mm_mul_ps(m0, m4x255f);

	__m128i m1 = _mm_cvtps_epi32(m0);

	if (m1.m128i_i32[3] > 255) m1.m128i_i32[3] = 255;
	if (m1.m128i_i32[2] > 255) m1.m128i_i32[2] = 255;
	if (m1.m128i_i32[1] > 255) m1.m128i_i32[1] = 255;

	return 0xff000000 | (m1.m128i_i32[3] << 16) | (m1.m128i_i32[2] << 8) | m1.m128i_i32[1];


	unsigned int red  (x * T(255));
	unsigned int green(y * T(255));
	unsigned int blue (z * T(255));

	return 0xff000000 | (blue << 16) | (green << 8) | red;
}*/

template<typename T>
T Vector3<T>::absolute_max(uint32_t& i) const {
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
Vector3<T> operator*(T s, const Vector3<T>& v) {
	return Vector3<T>(s * v.x, s * v.y, s * v.z);
}

template<typename T>
T dot(const Vector3<T>& a, const Vector3<T>& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<typename T>
T length(const Vector3<T>& v) {
	return std::sqrt(dot(v, v));
}

template<typename T>
T squared_length(const Vector3<T>& v) {
	return dot(v, v);
}

template<typename T>
Vector3<T> normalized(const Vector3<T>& v) {
	return v / length(v);
}

template<typename T>
Vector3<T> reciprocal(const Vector3<T>& v) {
	return Vector3<T>(T(1) / v.x, T(1) / v.y, T(1) / v.z);
}

template<typename T>
Vector3<T> cross(const Vector3<T>& a, const Vector3<T>& b) {
	return Vector3<T>(a.y * b.z - a.z * b.y,
					  a.z * b.x - a.x * b.z,
					  a.x * b.y - a.y * b.x);
}

template<typename T>
Vector3<T> project(const Vector3<T>& a, const Vector3<T>& b) {
	return dot(b, a) * b;
}

template<typename T>
T distance(const Vector3<T>& a, const Vector3<T>& b) {
	return length(a - b);
}

template<typename T>
T squared_distance(const Vector3<T>& a, const Vector3<T>& b) {
	return squared_length(a - b);
}

template<typename T>
Vector3<T> saturate(const Vector3<T>& v) {
	return Vector3<T>(std::min(std::max(v.x, T(0)), T(1)),
					  std::min(std::max(v.y, T(0)), T(1)),
					  std::min(std::max(v.z, T(0)), T(1)));
}

template<typename T>
Vector3<T> exp(const Vector3<T>& v) {
	return Vector3<T>(std::exp(v.x), std::exp(v.y), std::exp(v.z));
}

template<typename T>
Vector3<T> lerp(const Vector3<T>& a, const Vector3<T>& b, T t) {
	T u = T(1) - t;
	return u * a + t * b;
}

template<typename T>
Vector3<T> reflect(const Vector3<T>& normal, const Vector3<T>& v) {
	return T(2) * dot(v, normal) * normal - v;
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
Vector3<T> min(const Vector3<T>& a, const Vector3<T>& b) {
	return Vector3<T>(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

template<typename T>
Vector3<T> max(const Vector3<T>& a, const Vector3<T>& b) {
	return Vector3<T>(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}

template<typename T>
Vector3<T> abs(const Vector3<T>& v) {
	return Vector3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

template<typename T>
bool any_negative(const Vector3<T>& v) {
	return v.x < T(0) || v.y < T(0) || v.z < T(0);
}

template<typename T>
bool any_greater_one(const Vector3<T>& v) {
	return v.x > T(1) || v.y > T(1) || v.z > T(1);
}

template<typename T>
bool any_nan(const Vector3<T>& v) {
	return std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z);
}

template<typename T>
bool any_inf(const Vector3<T>& v) {
	return std::isinf(v.x) || std::isinf(v.y) || std::isinf(v.z);
}

/****************************************************************************
 *
 * Aligned 3D float vector
 *
 ****************************************************************************/

inline Vector3f_a::Vector3f_a() {}

inline constexpr Vector3f_a::Vector3f_a(float x, float y, float z) : v{x, y, z, 0.f} {}

inline Vector3f_a::Vector3f_a(const float* v) : v{v[0], v[1], v[2], 0.f} {}

inline Vector3f_a::Vector3f_a(float s) : v{s, s, s, 0.f} {}

inline Vector3f_a::Vector3f_a(Vector2<float> xy, float z) : v{xy.v[0], xy.v[1], z} {}

template<typename T>
Vector3f_a::Vector3f_a(const Vector3<T>& v) : v{float(v.x), float(v.y), float(v.z), 0.f} {}

inline Vector2<float> Vector3f_a::xy() const {
	return Vector2<float>(v[0], v[1]);
}

inline Vector3f_a Vector3f_a::operator+(float s) const {
	return Vector3f_a(v[0] + s, v[1] + s, v[2] + s);
}

inline Vector3f_a Vector3f_a::operator+(FVector3f_a a) const {
	return Vector3f_a(v[0] + a.v[0], v[1] + a.v[1], v[2] + a.v[2]);
}

inline Vector3f_a Vector3f_a::operator-(float s) const {
	return Vector3f_a(v[0] - s, v[1] - s, v[2] - s);
}

inline Vector3f_a Vector3f_a::operator-(FVector3f_a a) const {
	return Vector3f_a(v[0] - a.v[0], v[1] - a.v[1], v[2] - a.v[2]);
}

inline Vector3f_a Vector3f_a::operator*(FVector3f_a a) const {
	return Vector3f_a(v[0] * a.v[0], v[1] * a.v[1], v[2] * a.v[2]);
}

inline Vector3f_a Vector3f_a::operator/(float s) const {
	float is = 1.f / s;
	return Vector3f_a(is * v[0], is * v[1], is * v[2]);
}

inline Vector3f_a Vector3f_a::operator/(FVector3f_a a) const {
	return Vector3f_a(v[0] / a.v[0], v[1] / a.v[1], v[2] / a.v[2]);
}

inline Vector3f_a Vector3f_a::operator-() const {
	return Vector3f_a(-v[0], -v[1], -v[2]);
}

inline Vector3f_a& Vector3f_a::operator+=(FVector3f_a a) {
	v[0] += a.v[0];
	v[1] += a.v[1];
	v[2] += a.v[2];
	return *this;
}

inline Vector3f_a& Vector3f_a::operator-=(FVector3f_a a) {
	v[0] -= a.v[0];
	v[1] -= a.v[1];
	v[1] -= a.v[2];
	return *this;
}

inline Vector3f_a& Vector3f_a::operator*=(FVector3f_a a) {
	v[0] *= a.v[0];
	v[1] *= a.v[1];
	v[2] *= a.v[2];
	return *this;
}

inline Vector3f_a& Vector3f_a::operator*=(float s) {
	v[0] *= s;
	v[1] *= s;
	v[2] *= s;
	return *this;
}

inline Vector3f_a& Vector3f_a::operator/=(float s) {
	float is = 1.f / s;
	v[0] *= is;
	v[1] *= is;
	v[2] *= is;
	return *this;
}

inline bool Vector3f_a::operator==(FVector3f_a a) const {
	return v[0] == a.v[0] && v[1] == a.v[1] && v[2] == a.v[2];
}

inline bool Vector3f_a::operator!=(FVector3f_a a) const {
	return v[0] != a.v[0] || v[1] != a.v[1] || v[2] != a.v[2];
}

inline float Vector3f_a::absolute_max(uint32_t& i) const {
	float ax = std::abs(v[0]);
	float ay = std::abs(v[1]);
	float az = std::abs(v[2]);

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

inline Vector3f_a operator+(float s, FVector3f_a v) {
	return Vector3f_a(s + v.v[0], s + v.v[1], s + v.v[2]);
}

inline Vector3f_a operator-(float s, FVector3f_a v) {
	return Vector3f_a(s - v.v[0], s - v.v[1], s - v.v[2]);
}

inline Vector3f_a operator*(float s, FVector3f_a v) {
	return Vector3f_a(s * v.v[0], s * v.v[1], s * v.v[2]);
}

inline Vector3f_a operator/(float s, FVector3f_a v) {
	return Vector3f_a(s / v.v[0], s / v.v[1], s / v.v[2]);
}

inline float dot(FVector3f_a a, FVector3f_a b) {
	return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2];
}

inline float length(FVector3f_a v) {
	return std::sqrt(dot(v, v));
}

inline float squared_length(FVector3f_a v) {
	return dot(v, v);
}

inline Vector3f_a normalized(FVector3f_a v) {
	// This is slowest on both machines
//	return v / length(v);

	// This seems to give the best performance on clang & AMD
	return simd::rsqrt(dot(v, v)) * v;
}

inline Vector3f_a reciprocal(FVector3f_a v) {
//	return Vector3f_a(1.f / v.x, 1.f / v.y, 1.f / v.z);

	simd::Vector sx = simd::load_float3_unsafe(v);

	simd::Vector rcp = _mm_rcp_ps(sx);
	simd::Vector mul = _mm_mul_ps(sx, _mm_mul_ps(rcp, rcp));

	Vector3f_a result;
	simd::store_float3_unsafe(result, _mm_sub_ps(_mm_add_ps(rcp, rcp), mul));
	return result;
}

inline Vector3f_a cross(FVector3f_a a, FVector3f_a b) {
	return Vector3f_a(a.v[1] * b.v[2] - a.v[2] * b.v[1],
					  a.v[2] * b.v[0] - a.v[0] * b.v[2],
					  a.v[0] * b.v[1] - a.v[1] * b.v[0]);
}

inline Vector3f_a project(FVector3f_a a, FVector3f_a b) {
	return dot(b, a) * b;
}

inline float distance(FVector3f_a a, FVector3f_a b) {
	return length(a - b);
}

inline float squared_distance(FVector3f_a a, FVector3f_a b) {
	return squared_length(a - b);
}

inline Vector3f_a saturate(FVector3f_a v) {
	return Vector3f_a(std::min(std::max(v.v[0], 0.f), 1.f),
					  std::min(std::max(v.v[1], 0.f), 1.f),
					  std::min(std::max(v.v[2], 0.f), 1.f));
}

inline Vector3f_a exp(FVector3f_a v) {
	return Vector3f_a(std::exp(v.v[0]), std::exp(v.v[1]), std::exp(v.v[2]));
}

inline Vector3f_a pow(FVector3f_a v, float e) {
	return Vector3f_a(std::pow(v.v[0], e),
					  std::pow(v.v[1], e),
					  std::pow(v.v[2], e));
}

inline Vector3f_a lerp(FVector3f_a a, FVector3f_a b, float t) {
	float u = 1.f - t;
	return u * a + t * b;
}

inline Vector3f_a reflect(FVector3f_a normal, FVector3f_a v) {
	return 2.f * dot(v, normal) * normal - v;
}

inline void coordinate_system(FVector3f_a n, Vector3f_a& t, Vector3f_a& b) {
/*	Vector3f_a r1;

	if (n.x < 0.6f && n.x > -0.6f) {
		r1 = Vector3f_a(1.f, 0.f, 0.f);
	} else if (n.y < 0.6f && n.y > -0.6f) {
		r1 = Vector3f_a(0.f, 1.f, 0.f);
	} else {
		r1 = Vector3f_a(0.f, 0.f, 1.f);
	}

	Vector3f_a r0 = normalized(cross(n, r1));

	t = r0;
	b = cross(r0, n);
*/

	// https://gist.github.com/roxlu/3082114

	// Handle the singularity
	if (n.v[2] < -0.9999999f) {
		t = Vector3f_a( 0.f, -1.f, 0.f);
		b = Vector3f_a(-1.f,  0.f, 0.f);
		return;
	}

	const float c = 1.f / (1.f + n.v[2]);
	const float d = -n.v[0] * n.v[1] * c;
	t = Vector3f_a(1.f - n.v[0] * n.v[0] * c, d, -n.v[0]);
	b = Vector3f_a(d, 1.f - n.v[1] * n.v[1] * c, -n.v[1]);
}

inline Vector3f_a tangent(FVector3f_a n) {
/*	Vector3f_a r1;

	if (n.x < 0.6f && n.x > -0.6f) {
		r1 = Vector3f_a(1.f, 0.f, 0.f);
	} else if (n.y < 0.6f && n.y > -0.6f) {
		r1 = Vector3f_a(0.f, 1.f, 0.f);
	} else {
		r1 = Vector3f_a(0.f, 0.f, 1.f);
	}

	return normalized(cross(n, r1));
	*/

	// Handle the singularity
	if (n.v[2] < -0.9999999f) {
		return Vector3f_a(0.f, -1.f, 0.f);
	}

	const float c = 1.f / (1.f + n.v[2]);
	const float d = -n.v[0] * n.v[1] * c;
	return Vector3f_a(1.f - n.v[0] * n.v[0] * c, d, -n.v[0]);
}

inline Vector3f_a min(FVector3f_a a, FVector3f_a b) {
	return Vector3f_a(std::min(a.v[0], b.v[0]), std::min(a.v[1], b.v[1]), std::min(a.v[2], b.v[2]));
}

inline Vector3f_a max(FVector3f_a a, FVector3f_a b) {
	return Vector3f_a(std::max(a.v[0], b.v[0]), std::max(a.v[1], b.v[1]), std::max(a.v[2], b.v[2]));
}

inline float max_element(FVector3f_a v) {
	return std::max(std::max(v.v[0], v.v[1]), v.v[2]);
}

inline Vector3f_a abs(FVector3f_a v) {
	return Vector3f_a(std::abs(v.v[0]), std::abs(v.v[1]), std::abs(v.v[2]));
}

inline Vector3f_a cos(FVector3f_a v) {
	return Vector3f_a(std::cos(v.v[0]), std::cos(v.v[1]), std::cos(v.v[2]));
}

inline bool all_lesser(FVector3f_a v, float s) {
	return v.v[0] < s && v.v[1] < s && v.v[2] < s;
}

inline bool all_greater_equal(FVector3f_a v, float s) {
	return v.v[0] >= s && v.v[1] >= s && v.v[2] >= s;
}

inline bool any_negative(FVector3f_a v) {
	return v.v[0] < 0.f || v.v[1] < 0.f || v.v[2] < 0.f;
}

inline bool any_greater_zero(FVector3f_a v) {
	return v.v[0] > 0.f || v.v[1] > 0.f || v.v[2] > 0.f;
}

inline bool any_greater_one(FVector3f_a v) {
	return v.v[0] > 1.f || v.v[1] > 1.f || v.v[2] > 1.f;
}

inline bool any_lesser_one(FVector3f_a v) {
	return v.v[0] < 1.f || v.v[1] < 1.f || v.v[2] < 1.f;
}

inline bool any_nan(FVector3f_a v) {
	return std::isnan(v.v[0]) || std::isnan(v.v[1]) || std::isnan(v.v[2]);
}

inline bool any_inf(FVector3f_a v) {
	return std::isinf(v.v[0]) || std::isinf(v.v[1]) || std::isinf(v.v[2]);
}

inline bool all_finite(FVector3f_a v) {
	return std::isfinite(v.v[0]) && std::isfinite(v.v[1]) && std::isfinite(v.v[2]);
}

}
