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

  Vector3() = default;

  constexpr Vector3(T x, T y, T z) : v{x, y, z} {}

  explicit constexpr Vector3(T s);

  explicit constexpr Vector3(Vector2<T> xy, T z = T(0));

  explicit constexpr Vector3(T const* v);

  explicit constexpr Vector3(Vector3f_a const& a);

  template <typename U>
  explicit constexpr Vector3(Vector3<U> const& a);

  constexpr Vector2<T> xy() const;

  constexpr T operator[](uint32_t i) const;
  constexpr T& operator[](uint32_t i);

  Vector3 operator+(T s) const;

  Vector3 operator+(Vector3 const& a) const;

  Vector3 operator-(T s) const;

  Vector3 operator-(Vector3 const& v) const;

  Vector3 operator*(Vector3 const& a) const;

  Vector3 operator/(T s) const;

  Vector3 operator/(Vector3 const& v) const;

  Vector3 operator-() const;

  Vector3& operator+=(Vector3 const& a);

  Vector3& operator-=(Vector3 const& a);

  Vector3& operator*=(Vector3 const& a);

  Vector3& operator*=(T s);

  Vector3& operator/=(T s);

  bool operator==(Vector3 const& a) const;

  bool operator!=(Vector3 const& a) const;

  //	explicit operator unsigned int() const;

  T absolute_max(uint32_t& i) const;

  static constexpr Vector3 identity();
};

//==============================================================================
// Aligned 3D float vector
//==============================================================================

struct alignas(16) Vector3f_a {
  // 4 instead of 3 in order to hide pad warning
  float v[4];

  Vector3f_a() = default;

  constexpr Vector3f_a(float x, float y, float z) : v{x, y, z, 0.f} {}

  constexpr Vector3f_a(float const* a);

  explicit constexpr Vector3f_a(float s);

  explicit constexpr Vector3f_a(Vector2<float> const xy, float z);

  template <typename T>
  explicit constexpr Vector3f_a(Vector3<T> const& a);

  constexpr Vector2<float> xy() const;

  constexpr float operator[](uint32_t i) const;
  constexpr float& operator[](uint32_t i);

  float absolute_max(uint32_t& i) const;

  static constexpr Vector3f_a identity();
};

}  // namespace math

#endif
