#ifndef SU_BASE_MATH_VECTOR4_HPP
#define SU_BASE_MATH_VECTOR4_HPP

#include "vector.hpp"

namespace math {

//==============================================================================
// Generic 4D vector
//==============================================================================

template <typename T>
struct Vector4 {
    T v[4];

    Vector4() = default;

    constexpr Vector4(T x, T y, T z, T w = T(1));

    explicit constexpr Vector4(T s);

    explicit constexpr Vector4(Vector2<T> xy, T z, T w = T(1));

    explicit constexpr Vector4(Vector3<T> xyz, T w = T(1));

    explicit constexpr Vector4(Vector4f_a_p a);

    constexpr Vector2<T> xy() const;
    constexpr Vector3<T> xyz() const;

    constexpr T  operator[](uint32_t i) const;
    constexpr T& operator[](uint32_t i);

    constexpr Vector4 operator+(Vector4 const& v) const;

    constexpr Vector4 operator*(Vector4 const& v) const;

    constexpr Vector4 operator/(T s) const;

    constexpr Vector4& operator+=(Vector4 const& v);

    constexpr Vector4& operator-=(Vector4 const& v);
};

//==============================================================================
// Aligned 4D float vector
//==============================================================================

struct alignas(16) Vector4f_a {
    float v[4];

    Vector4f_a() = default;

    constexpr Vector4f_a(float x, float y, float z, float w = 1.f);

    explicit constexpr Vector4f_a(float s);

    explicit constexpr Vector4f_a(Vector2<float> const xy, float z, float w = 1.f);

    explicit constexpr Vector4f_a(Vector3f_a_p xyz, float w = 1.f);

    explicit constexpr Vector4f_a(Vector3<float> const& xyz, float w = 1.f);

    explicit constexpr Vector4f_a(float const* a, float w = 1.f);

    template <typename T>
    explicit constexpr Vector4f_a(Vector4<T> const& a);

    explicit Vector4f_a(Simdf_p o);

    Vector2<float> constexpr xy() const;

    Vector2<float> constexpr zw() const;

    Vector3f_a constexpr xyz() const;

    float constexpr  operator[](uint32_t i) const;
    float constexpr& operator[](uint32_t i);
};

//==============================================================================
// Aligned 4D int vector
//==============================================================================

struct alignas(16) Vector4i_a {
    int32_t v[4];

    Vector4i_a() = default;

    constexpr Vector4i_a(int32_t x, int32_t y, int32_t z, int32_t w);

    constexpr Vector4i_a(Vector2<int32_t> xy, Vector2<int32_t> zw);

    explicit constexpr Vector4i_a(int32_t s) : v{s, s, s, s} {}

    constexpr Vector2<int32_t> xy() const;

    constexpr Vector2<int32_t> zw() const;

    constexpr int32_t operator[](uint32_t i) const;

    constexpr int32_t& operator[](uint32_t i);
};

}  // namespace math

#endif
