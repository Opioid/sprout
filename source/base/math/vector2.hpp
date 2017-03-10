#pragma once

#include <cstdint>

namespace math {

template<typename T>
struct Vector2 {
	T v[2];

	Vector2() = default;

	Vector2(T s) : v{s, s} {}

	Vector2(T x, T y) : v{x, y} {}

	template<typename U>
	explicit Vector2(Vector2<U> v) : v{T(v[0]), T(v[1])} {}

	Vector2 yx() const {
		return Vector2(v[1], v[0]);
	}

	T operator[](uint32_t i) const {
		return v[i];
	}

	T& operator[](uint32_t i) {
		return v[i];
	}

	static constexpr Vector2 identity() {
		return Vector2(T(0), T(0));
	}
};

}

using byte2  = math::Vector2<uint8_t>;
using float2 = math::Vector2<float>;
using int2   = math::Vector2<int32_t>;
using uint2  = math::Vector2<uint32_t>;
