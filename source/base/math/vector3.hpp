#ifndef SU_BASE_MATH_VECTOR3_HPP
#define SU_BASE_MATH_VECTOR3_HPP

#include "simd/simd.hpp"
#include "vector.hpp"

#include <cstdint>

namespace math {

struct Vector3f_a;

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

    explicit constexpr Vector3(Vector3f_a const& a);

    template <typename U>
    explicit constexpr Vector3(Vector3<U> const& a);

    template <typename U>
    explicit constexpr Vector3(U x, U y, U z) : v{T(x), T(y), T(z)} {}

    constexpr Vector2<T> xy() const;

    constexpr T  operator[](uint32_t i) const;
    constexpr T& operator[](uint32_t i);

    Vector3 operator+(T s) const;

    Vector3 operator+(Vector3 const& a) const;

    Vector3 operator-(T s) const;

    Vector3 operator-(Vector3 const& v) const;

    Vector3 operator*(Vector3 const& a) const;

    Vector3 operator/(T s) const;

    Vector3 operator/(Vector3 const& v) const;

    constexpr Vector3 operator<<(uint32_t c) const;

    constexpr Vector3 operator>>(uint32_t c) const;

    Vector3 operator-() const;

    Vector3& operator+=(Vector3 const& a);

    Vector3& operator-=(Vector3 const& a);

    Vector3& operator*=(Vector3 const& a);

    Vector3& operator*=(T s);

    Vector3& operator/=(T s);

    bool operator==(Vector3 const& a) const;

    bool operator!=(Vector3 const& a) const;
};

//==============================================================================
// Aligned 3D float vector
//==============================================================================

struct Simd3f;

struct alignas(16) Vector3f_a {
    // 4 instead of 3 in order to hide pad warning
    float v[4];

    Vector3f_a() = default;

    constexpr Vector3f_a(float x, float y, float z) : v{x, y, z, 0.f} {}

    explicit constexpr Vector3f_a(float const* a);

    explicit constexpr Vector3f_a(float s);

    explicit constexpr Vector3f_a(Vector2<float> const xy, float z = 0.f);

    template <typename T>
    explicit constexpr Vector3f_a(Vector3<T> const& a);

    explicit Vector3f_a(Simd3f const& o);

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

    explicit Simd3f(Simd1f const& s);

    Simd3f(float sx, float sy);

    Simd3f(float sx, float sy, float sz);

    explicit Simd3f(float const* a);

    explicit Simd3f(Vector3f_a const& o);

    static Simd3f create_from_3(float const* f);

    static Simd3f create_from_3_unaligned(float const* f);

    float x() const;
    float y() const;
    float z() const;
    float w() const;

    Simd3f splat_x() const;

    __m128 v;
};

}  // namespace math

#endif
