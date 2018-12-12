#ifndef SU_BASE_MATH_VECTOR3_HPP
#define SU_BASE_MATH_VECTOR3_HPP

#include <algorithm>
#include <cmath>
#include <cstdint>
#include "vector.hpp"

namespace math {

struct Vector3f_a;

//==============================================================================
// Generic 3D vector
//==============================================================================

template <typename T>
struct Vector3 {
    T v[3];

    Vector3() noexcept = default;

    constexpr Vector3(T x, T y, T z) noexcept : v{x, y, z} {}

    explicit constexpr Vector3(T s) noexcept;

    explicit constexpr Vector3(Vector2<T> xy, T z = T(0)) noexcept;

    explicit constexpr Vector3(T const* a) noexcept;

    explicit constexpr Vector3(Vector3f_a const& a) noexcept;

    template <typename U>
    explicit constexpr Vector3(Vector3<U> const& a) noexcept;

    template <typename U>
    explicit constexpr Vector3(U x, U y, U z) noexcept : v{T(x), T(y), T(z)} {}

    constexpr Vector2<T> xy() const noexcept;

    constexpr T  operator[](uint32_t i) const noexcept;
    constexpr T& operator[](uint32_t i) noexcept;

    Vector3 operator+(T s) const noexcept;

    Vector3 operator+(Vector3 const& a) const noexcept;

    Vector3 operator-(T s) const noexcept;

    Vector3 operator-(Vector3 const& v) const noexcept;

    Vector3 operator*(Vector3 const& a) const noexcept;

    Vector3 operator/(T s) const noexcept;

    Vector3 operator/(Vector3 const& v) const noexcept;

    constexpr Vector3 operator<<(uint32_t c) const noexcept;

    constexpr Vector3 operator>>(uint32_t c) const noexcept;

    Vector3 operator-() const noexcept;

    Vector3& operator+=(Vector3 const& a) noexcept;

    Vector3& operator-=(Vector3 const& a) noexcept;

    Vector3& operator*=(Vector3 const& a) noexcept;

    Vector3& operator*=(T s) noexcept;

    Vector3& operator/=(T s) noexcept;

    bool operator==(Vector3 const& a) const noexcept;

    bool operator!=(Vector3 const& a) const noexcept;

    //	explicit operator unsigned int() const;

    T absolute_max(uint32_t& i) const noexcept;
};

//==============================================================================
// Aligned 3D float vector
//==============================================================================

struct alignas(16) Vector3f_a {
    // 4 instead of 3 in order to hide pad warning
    float v[4];

    Vector3f_a() noexcept = default;

    constexpr Vector3f_a(float x, float y, float z) noexcept : v{x, y, z, 0.f} {}

    constexpr Vector3f_a(float const* a) noexcept;

    explicit constexpr Vector3f_a(float s) noexcept;

    explicit constexpr Vector3f_a(Vector2<float> const xy, float z = 0.f) noexcept;

    template <typename T>
    explicit constexpr Vector3f_a(Vector3<T> const& a) noexcept;

    constexpr Vector2<float> xy() const noexcept;

    float constexpr  operator[](uint32_t i) const noexcept;
    float constexpr& operator[](uint32_t i) noexcept;

    float absolute_max(uint32_t& i) const noexcept;
};

}  // namespace math

#endif
