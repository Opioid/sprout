#pragma once

#include "vector2.hpp"
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace math {

struct Vector3f_a;

/****************************************************************************
 *
 * Generic 3D vector
 *
 ****************************************************************************/

template<typename T>
struct Vector3 {
	T v[3];

	Vector3() = default;

	constexpr Vector3(T x, T y, T z) : v{x, y, z} {}

	explicit constexpr Vector3(T s);

	explicit constexpr Vector3(Vector2<T> xy, T z = T(0));

	explicit constexpr Vector3(const T* v);

	explicit constexpr Vector3(const Vector3f_a& a);

	constexpr Vector2<T> xy() const;

	constexpr T operator[](uint32_t i) const;
	constexpr T& operator[](uint32_t i);

	Vector3 operator+(T s) const;

	Vector3 operator+(const Vector3& a) const;

	Vector3 operator-(T s) const;

	Vector3 operator-(const Vector3& v) const;
		
	Vector3 operator*(const Vector3& a) const;
		
	Vector3 operator/(T s) const;

	Vector3 operator/(const Vector3& v) const;

	Vector3 operator-() const;

	Vector3& operator+=(const Vector3& a);

	Vector3& operator-=(const Vector3& a);

	Vector3& operator*=(const Vector3& a);

	Vector3& operator*=(T s);

	Vector3& operator/=(T s);

	bool operator==(const Vector3& a) const;

	bool operator!=(const Vector3& a) const;

//	explicit operator unsigned int() const;

	T absolute_max(uint32_t& i) const;

	static const Vector3 identity;
};

/****************************************************************************
 *
 * Aligned 3D float vector
 *
 ****************************************************************************/

struct alignas(16) Vector3f_a {
	// 4 instead of 3 in order to hide pad warning
	float v[4];

	Vector3f_a() = default;

	constexpr Vector3f_a(float x, float y, float z);

	constexpr Vector3f_a(const float* v);

	explicit constexpr Vector3f_a(float s);

	explicit constexpr Vector3f_a(const Vector2<float> xy, float z);

	template<typename T>
	explicit constexpr Vector3f_a(const Vector3<T>& v);

	constexpr Vector2<float> xy() const;

	constexpr float operator[](uint32_t i) const;
	constexpr float& operator[](uint32_t i);

	float absolute_max(uint32_t& i) const;

	static constexpr Vector3f_a identity() ;
};

}

using byte3  = math::Vector3<uint8_t>;
// using float3 = Vector3<float>;
using float3 = math::Vector3f_a;
using int3   = math::Vector3<int32_t>;
using uint3  = math::Vector3<uint32_t>;
using packed_float3 = math::Vector3<float>;
