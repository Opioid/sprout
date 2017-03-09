#pragma once

#include "vector2.hpp"
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace math {

struct Vector3f_a;

using FVector3f_a = const Vector3f_a&;
//using FVector3f_a = Vector3f_a;

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

	explicit Vector3(FVector3f_a a);

	Vector2<T> xy() const;

	T operator[](uint32_t i) const;
	T& operator[](uint32_t i);

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
bool any_negative(const Vector3<T>& v);

template<typename T>
bool any_greater_one(const Vector3<T>& v);

template<typename T>
bool any_nan(const Vector3<T>& v);

template<typename T>
bool any_inf(const Vector3<T>& v);

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

	Vector3f_a(const float* v) : v{v[0], v[1], v[2], 0.f} {}

	explicit constexpr Vector3f_a(float s) : v{s, s, s, 0.f} {}

	explicit Vector3f_a(Vector2<float> xy, float z) : v{xy[0], xy[1], z, 0.f} {}

	template<typename T>
	explicit Vector3f_a(const Vector3<T>& v) : v{float(v[0]), float(v[1]), float(v[2]), 0.f} {}

	Vector2<float> xy() const {
		return Vector2<float>(v[0], v[1]);
	}

	float operator[](uint32_t i) const{
		return v[i];
	}

	float& operator[](uint32_t i) {
		return v[i];
	}

	bool operator==(FVector3f_a a) const {
		return v[0] == a[0] && v[1] == a[1] && v[2] == a[2];
	}

	bool operator!=(FVector3f_a a) const {
		return v[0] != a[0] || v[1] != a[1] || v[2] != a[2];
	}

	float absolute_max(uint32_t& i) const {
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

using float3_p = math::FVector3f_a;
