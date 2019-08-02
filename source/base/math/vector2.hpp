#ifndef SU_BASE_MATH_VECTOR2_HPP
#define SU_BASE_MATH_VECTOR2_HPP

#include <cstdint>
#include "vector.hpp"

namespace math {

template <typename T>
struct Vector2 {
    T v[2];

    Vector2() noexcept;

    explicit constexpr Vector2(T s) noexcept;

    constexpr Vector2(T x, T y) noexcept : v{x, y} {}

    template <typename U>
    explicit constexpr Vector2(U x, U y) noexcept;

    template <typename U>
    explicit constexpr Vector2(Vector2<U> v) noexcept;

    constexpr Vector2 yx() const noexcept;

    constexpr T operator[](uint32_t i) const noexcept;

    constexpr T& operator[](uint32_t i) noexcept;

    static constexpr Vector2 identity() noexcept;
};

}  // namespace math

#endif
