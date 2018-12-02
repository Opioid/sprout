#ifndef SU_BASE_MATH_VECTOR2_HPP
#define SU_BASE_MATH_VECTOR2_HPP

#include <cstdint>
#include "vector.hpp"

namespace math {

template <typename T>
struct Vector2 {
    T v[2];

    Vector2() = default;

    constexpr Vector2(T s);

    constexpr Vector2(T x, T y);

    template <typename U>
    explicit constexpr Vector2(U x, U y);

    template <typename U>
    explicit constexpr Vector2(Vector2<U> v);

    constexpr Vector2 yx() const;

    constexpr T operator[](uint32_t i) const;

    constexpr T& operator[](uint32_t i);

    static constexpr Vector2 identity();
};

}  // namespace math

#endif
