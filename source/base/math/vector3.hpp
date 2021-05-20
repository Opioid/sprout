#ifndef SU_BASE_MATH_VECTOR3_HPP
#define SU_BASE_MATH_VECTOR3_HPP

#include "simd/simd.hpp"
#include "vector.hpp"

namespace math {

//==============================================================================
// Generic 3D vector
//==============================================================================

template <typename T>
struct Vector3 {
    T v[3];

    Vector3() = default;

    constexpr Vector3(T x, T y, T z) : v{x, y, z} {}

    explicit constexpr Vector3(T s);

    explicit constexpr Vector3(Vector2<T> xy, T z = T(0));

    explicit constexpr Vector3(T const* a);

    explicit constexpr Vector3(Vector3f_a_p a);

    template <typename U>
    explicit constexpr Vector3(Vector3<U> a);

    template <typename U>
    explicit constexpr Vector3(U x, U y, U z) : v{T(x), T(y), T(z)} {}

    constexpr Vector2<T> xy() const;

    constexpr T  operator[](uint32_t i) const;
    constexpr T& operator[](uint32_t i);

    Vector3 operator+(T s) const;

    Vector3 operator+(Vector3 a) const;

    Vector3 operator-(T s) const;

    Vector3 operator-(Vector3 v) const;

    Vector3 operator*(Vector3 a) const;

    Vector3 operator/(T s) const;

    Vector3 operator/(Vector3 v) const;

    constexpr Vector3 operator<<(uint32_t c) const;

    constexpr Vector3 operator>>(uint32_t c) const;

    Vector3 operator-() const;

    Vector3& operator+=(Vector3 a);

    Vector3& operator-=(Vector3 a);

    Vector3& operator*=(Vector3 a);

    Vector3& operator*=(T s);

    Vector3& operator/=(T s);

    bool operator==(Vector3 a) const;

    bool operator!=(Vector3 a) const;
};

//==============================================================================
// Aligned 3D float vector
//==============================================================================

template <typename T>
struct Vector3_a {
    T v[4];

    Vector3_a() = default;

    constexpr Vector3_a(T x, T y, T z) : v{x, y, z, T(0)} {}

    constexpr Vector2<T> xy() const;

    constexpr T  operator[](uint32_t i) const;
    constexpr T& operator[](uint32_t i);
};

struct alignas(16) Vector3f_a {
    float v[4];

    Vector3f_a() = default;

    constexpr Vector3f_a(float x, float y, float z) : v{x, y, z, 0.f} {}

    constexpr Vector3f_a(float x, float y, float z, float w) : v{x, y, z, w} {}

    constexpr Vector3f_a(Vector3f_a o, float w) : v{o[0], o[1], o[2], w} {}

    explicit constexpr Vector3f_a(float const* a);

    explicit constexpr Vector3f_a(float s);

    explicit constexpr Vector3f_a(Vector2<float> const xy, float z = 0.f);

    template <typename T>
    explicit constexpr Vector3f_a(Vector3<T> a);

    explicit Vector3f_a(Simd3f_p o);

    constexpr Vector2<float> xy() const;

    float constexpr  operator[](uint32_t i) const;
    float constexpr& operator[](uint32_t i);
};

//==============================================================================
// SIMD 3D float vector
//==============================================================================

struct Simd3f {
    Simd3f() = default;

    Simd3f(__m128 m);

    explicit Simd3f(float s);

    explicit Simd3f(Simd1f_p s);

    Simd3f(float sx, float sy);

    Simd3f(float sx, float sy, float sz);

    explicit Simd3f(float const* a);

    explicit Simd3f(Vector3f_a_p o);

    static Simd3f create_from_3(float const* f);

    static Simd3f create_from_3_unaligned(float const* f);

    float x() const;
    float y() const;
    float z() const;
    float w() const;

    Simd3f splat_x() const;
    Simd3f splat_w() const;

    __m128 v;
};

}  // namespace math

#endif
