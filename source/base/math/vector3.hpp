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

	Vector3();

	Vector3(T x, T y, T z);

	explicit Vector3(T s);

	explicit Vector3(Vector2<T> xy, T z = T(0));

	explicit Vector3(const T* v);

	explicit Vector3(const Vector3f_a& a);

	Vector2<T> xy() const;

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

	constexpr Vector3f_a(float x, float y, float z) : v{x, y, z, 0.f} {}

	constexpr Vector3f_a(const float* v) : v{v[0], v[1], v[2], 0.f} {}

	explicit constexpr Vector3f_a(float s) : v{s, s, s, 0.f} {}

	explicit constexpr Vector3f_a(const Vector2<float> xy, float z) : v{xy[0], xy[1], z, 0.f} {}

	template<typename T>
	explicit constexpr Vector3f_a(const Vector3<T>& v) :
		v{float(v[0]), float(v[1]), float(v[2]), 0.f} {}

	constexpr Vector2<float> xy() const {
		return Vector2<float>(v[0], v[1]);
	}

	constexpr float operator[](uint32_t i) const{
		return v[i];
	}

	constexpr float& operator[](uint32_t i) {
		return v[i];
	}

	constexpr float absolute_max(uint32_t& i) const {
		const float ax = std::abs(v[0]);
		const float ay = std::abs(v[1]);
		const float az = std::abs(v[2]);

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

	static constexpr Vector3f_a identity() {
		return Vector3f_a(0.f, 0.f, 0.f);
	}
};

}

using byte3  = math::Vector3<uint8_t>;
// using float3 = Vector3<float>;
using float3 = math::Vector3f_a;
using int3  = math::Vector3<int32_t>;
using uint3  = math::Vector3<uint32_t>;
using packed_float3 = math::Vector3<float>;
