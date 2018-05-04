#ifndef SU_BASE_MATH_VECTOR4_HPP
#define SU_BASE_MATH_VECTOR4_HPP

#include "vector.hpp"
#include <cstdint>

namespace math {

struct Vector4f_a;
struct Vector4i_a;

//==============================================================================
// Generic 4D vector
//==============================================================================

template<typename T>
struct Vector4 {
	T v[4];

	Vector4() = default;

	constexpr Vector4(T x, T y, T z, T w = T(1));

	explicit constexpr Vector4(T s);

	explicit constexpr Vector4(Vector2<T> xy, T z, T w = T(1));

	explicit constexpr Vector4(Vector3<T> const& xyz, T w = T(1));

	constexpr Vector3<T> xyz() const;

	constexpr T operator[](uint32_t i) const;
	constexpr T& operator[](uint32_t i);

	constexpr Vector4 operator+(Vector4 const& v) const;

	constexpr Vector4 operator*(Vector4 const& v) const;

	constexpr Vector4 operator/(T s) const;

	constexpr Vector4& operator+=(Vector4 const& v);

	constexpr Vector4& operator-=(Vector4 const& v);

	static constexpr Vector4 identity();
};

//==============================================================================
// Aligned 4D float vector
//==============================================================================

struct alignas(16) Vector4f_a {
	float v[4];

	Vector4f_a() = default;

	constexpr Vector4f_a(float x, float y, float z, float w = 1.f);

	explicit constexpr Vector4f_a(float s);

	explicit constexpr Vector4f_a(const Vector2<float> xy, float z, float w = 1.f);

	explicit constexpr Vector4f_a(FVector3f_a xyz, float w = 1.f);

	explicit constexpr Vector4f_a(const Vector3<float>& xyz, float w = 1.f);

	constexpr Vector3f_a xyz() const;

	constexpr float operator[](uint32_t i) const;
	constexpr float& operator[](uint32_t i);

	static constexpr Vector4f_a identity();
};

//==============================================================================
// Aligned 4D int vector
//==============================================================================

struct alignas(16) Vector4i_a {
	int32_t v[4];

	Vector4i_a() = default;

	constexpr Vector4i_a(int32_t x, int32_t y, int32_t z, int32_t w);

	constexpr Vector4i_a(Vector2<int32_t> xy, Vector2<int32_t> zw);

	explicit constexpr Vector4i_a(int32_t s);

	constexpr Vector2<int32_t> xy() const;

	constexpr Vector2<int32_t> zw() const;

	constexpr int32_t operator[](uint32_t i) const;

	constexpr int32_t& operator[](uint32_t i);
};

}

#endif
