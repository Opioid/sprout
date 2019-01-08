#ifndef SU_BASE_MATH_VECTOR4_HPP
#define SU_BASE_MATH_VECTOR4_HPP

#include <cstdint>
#include "vector.hpp"

namespace math {

struct Vector4f_a;
struct Vector4i_a;

//==============================================================================
// Generic 4D vector
//==============================================================================

template <typename T>
struct Vector4 {
    T v[4];

    Vector4() noexcept = default;

    constexpr Vector4(T x, T y, T z, T w = T(1)) noexcept;

    explicit constexpr Vector4(T s) noexcept;

    explicit constexpr Vector4(Vector2<T> xy, T z, T w = T(1)) noexcept;

    explicit constexpr Vector4(Vector3<T> const& xyz, T w = T(1)) noexcept;

    explicit constexpr Vector4(Vector4f_a const& a) noexcept;

    constexpr Vector2<T> xy() const noexcept;
    constexpr Vector3<T> xyz() const noexcept;

    constexpr T  operator[](uint32_t i) const noexcept;
    constexpr T& operator[](uint32_t i) noexcept;

    constexpr Vector4 operator+(Vector4 const& v) const noexcept;

    constexpr Vector4 operator*(Vector4 const& v) const noexcept;

    constexpr Vector4 operator/(T s) const noexcept;

    constexpr Vector4& operator+=(Vector4 const& v) noexcept;

    constexpr Vector4& operator-=(Vector4 const& v) noexcept;
};

//==============================================================================
// Aligned 4D float vector
//==============================================================================

struct alignas(16) Vector4f_a {
    float v[4];

    Vector4f_a() noexcept = default;

    constexpr Vector4f_a(float x, float y, float z, float w = 1.f) noexcept;

    explicit constexpr Vector4f_a(float s) noexcept;

    explicit constexpr Vector4f_a(Vector2<float> const xy, float z, float w = 1.f) noexcept;

    explicit constexpr Vector4f_a(Vector3f_a const& xyz, float w = 1.f) noexcept;

    explicit constexpr Vector4f_a(Vector3<float> const& xyz, float w = 1.f) noexcept;

    template <typename T>
    explicit constexpr Vector4f_a(Vector4<T> const& a) noexcept;

    Vector3f_a constexpr xyz() const noexcept;

    float constexpr  operator[](uint32_t i) const noexcept;
    float constexpr& operator[](uint32_t i) noexcept;
};

//==============================================================================
// Aligned 4D int vector
//==============================================================================

struct alignas(16) Vector4i_a {
    int32_t v[4];

    Vector4i_a() noexcept = default;

    constexpr Vector4i_a(int32_t x, int32_t y, int32_t z, int32_t w) noexcept;

    constexpr Vector4i_a(Vector2<int32_t> xy, Vector2<int32_t> zw) noexcept;

    explicit constexpr Vector4i_a(int32_t s) noexcept;

    constexpr Vector2<int32_t> xy() const noexcept;

    constexpr Vector2<int32_t> zw() const noexcept;

    constexpr int32_t operator[](uint32_t i) const noexcept;

    constexpr int32_t& operator[](uint32_t i) noexcept;
};

}  // namespace math

#endif
