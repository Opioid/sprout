#pragma once

#include "vector3.hpp"
#include <cstdint>

namespace math {

struct Vector4f_a;
using FVector4f_a = const Vector4f_a&;

struct Vector4i_a;
using FVector4i_a = const Vector4i_a&;

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

	T operator[](uint32_t i) const;
	T& operator[](uint32_t i);

	Vector4 operator+(const Vector4& v) const;

	Vector4 operator*(const Vector4& v) const;

	Vector4 operator/(T s) const;

	Vector4& operator+=(const Vector4& v);

	Vector4& operator-=(const Vector4& v);

	static constexpr Vector4 identity();
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

	Vector4f_a() = default;

	constexpr Vector4f_a(float x, float y, float z, float w = 1.f) :
		v{x, y, z, w} {}

	explicit Vector4f_a(float s) : v{s, s, s, s} {}

	explicit Vector4f_a(Vector2<float> xy, float z, float w = 1.f) :
		v{xy[0], xy[1], z, w} {}

	explicit Vector4f_a(const Vector3f_a& xyz, float w = 1.f) :
		v{xyz[0], xyz[1], xyz[2], w} {}

	explicit Vector4f_a(const Vector3<float>& xyz, float w = 1.f) :
		v{xyz[0], xyz[1], xyz[2], w} {}

	Vector3f_a xyz() const {
		return Vector3f_a(v);
	}

	float operator[](uint32_t i) const {
		return v[i];
	}

	float& operator[](uint32_t i) {
		return v[i];
	}

	static constexpr Vector4f_a identity() {
		return Vector4f_a(0.f, 0.f, 0.f, 0.f);
	}
};

/****************************************************************************
 *
 * Aligned 4D int vector
 *
 ****************************************************************************/

struct alignas(16) Vector4i_a {
	int32_t v[4];

	Vector4i_a() = default;

	Vector4i_a(int32_t x, int32_t y, int32_t z, int32_t w) :
		v{x, y, z, w} {}

	Vector4i_a(Vector2<int32_t> xy, Vector2<int32_t> zw) :
		v{xy[0], xy[1], zw[0], zw[1]} {}

	explicit Vector4i_a(int32_t s) : v{s, s, s, s} {}

	const Vector2<int32_t> xy() const {
		return Vector2<int32_t>(v[0], v[1]);
	}

	const Vector2<int32_t> zw() const {
		return Vector2<int32_t>(v[2], v[3]);
	}

	int32_t operator[](uint32_t i) const {
		return v[i];
	}

	int32_t& operator[](uint32_t i) {
		return v[i];
	}
};

}

using byte4  = math::Vector4<uint8_t>;
using short4 = math::Vector4<int16_t>;
// using float4 = Vector4<float>;
using float4 = math::Vector4f_a;
using int4   = math::Vector4i_a;

using float4_p = math::FVector4f_a;
