#pragma once

#include <cstdint>

namespace math {

template<typename T>
struct Vector2 {
	T v[2];

	Vector2() = default;

	constexpr Vector2(T s);

	constexpr Vector2(T x, T y);

	template<typename U>
	explicit constexpr Vector2(Vector2<U> v);

	constexpr Vector2 yx() const;

	constexpr T operator[](uint32_t i) const;

	constexpr T& operator[](uint32_t i);

	static constexpr Vector2 identity();
};

}

using byte2  = math::Vector2<uint8_t>;
using float2 = math::Vector2<float>;
using int2   = math::Vector2<int32_t>;
using uint2  = math::Vector2<uint32_t>;
