#ifndef SU_BASE_MATH_VECTOR4_INL
#define SU_BASE_MATH_VECTOR4_INL

#include "vector3.inl"
#include "vector4.hpp"

namespace math {

//==============================================================================
// Generic 4D vector
//==============================================================================

template <typename T>
constexpr Vector4<T>::Vector4(T x, T y, T z, T w) : v{x, y, z, w} {}

template <typename T>
constexpr Vector4<T>::Vector4(T s) : v{s, s, s, s} {}

template <typename T>
constexpr Vector4<T>::Vector4(Vector2<T> xy, T z, T w) : v{xy[0], xy[1], z, w} {}

template <typename T>
constexpr Vector4<T>::Vector4(Vector3<T> xyz, T w) : v{xyz[0], xyz[1], xyz[2], w} {}

template <typename T>
constexpr Vector4<T>::Vector4(Vector4f_a_p a) : v{T(a[0]), T(a[1]), T(a[2]), T(a[3])} {}

template <typename T>
constexpr Vector2<T> Vector4<T>::xy() const {
    return Vector2<T>(v);
}

template <typename T>
constexpr Vector3<T> Vector4<T>::xyz() const {
    return Vector3<T>(v);
}

template <typename T>
constexpr T Vector4<T>::operator[](uint32_t i) const {
    return v[i];
}

template <typename T>
constexpr T& Vector4<T>::operator[](uint32_t i) {
    return v[i];
}

template <typename T>
constexpr Vector4<T> Vector4<T>::operator+(Vector4 const& v) const {
    return Vector4(v[0] + v[0], v[1] + v[1], v[2] + v[2], v[3] + v[3]);
}

template <typename T>
constexpr Vector4<T> Vector4<T>::operator*(Vector4 const& v) const {
    return Vector4(v[0] * v[0], v[1] * v[1], v[2] * v[2], v[3] * v[3]);
}

template <typename T>
constexpr Vector4<T> Vector4<T>::operator/(T s) const {
    T is = T(1) / s;
    return Vector4(is * v[0], is * v[1], is * v[2], is * v[2]);
}

template <typename T>
constexpr Vector4<T>& Vector4<T>::operator+=(Vector4 const& a) {
    v[0] += a[0];
    v[1] += a[1];
    v[2] += a[2];
    v[3] += a[3];
    return *this;
}

template <typename T>
constexpr Vector4<T>& Vector4<T>::operator-=(Vector4 const& a) {
    v[0] -= a[0];
    v[1] -= a[1];
    v[2] -= a[2];
    v[3] -= a[3];
    return *this;
}

template <typename T>
static constexpr bool operator==(const Vector4<T>& a, const Vector4<T>& b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

template <typename T>
static constexpr bool operator!=(const Vector4<T>& a, const Vector4<T>& b) {
    return a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3];
}

template <typename T>
constexpr Vector4<T> operator*(T s, const Vector4<T>& v) {
    return Vector4<T>(s * v[0], s * v[1], s * v[2], s * v[3]);
}

template <typename T>
constexpr T dot(const Vector4<T>& a, const Vector4<T>& b) {
    return (a[0] * b[0] + a[1] * b[1]) + (a[2] * b[2] + a[3] * b[3]);
}

//==============================================================================
// Aligned 4D float vector
//==============================================================================

inline constexpr Vector4f_a::Vector4f_a(float x, float y, float z, float w) : v{x, y, z, w} {}

inline constexpr Vector4f_a::Vector4f_a(float s) : v{s, s, s, s} {}

inline constexpr Vector4f_a::Vector4f_a(Vector2<float> const xy, float z, float w)
    : v{xy[0], xy[1], z, w} {}

inline constexpr Vector4f_a::Vector4f_a(Vector3f_a_p xyz, float w) : v{xyz[0], xyz[1], xyz[2], w} {}

inline constexpr Vector4f_a::Vector4f_a(Vector3<float> const& xyz, float w)
    : v{xyz[0], xyz[1], xyz[2], w} {}

inline constexpr Vector4f_a::Vector4f_a(float const* a, float w) : v{a[0], a[1], a[2], w} {}

inline Vector4f_a::Vector4f_a(Simdf_p o) {
    _mm_store_ps(v, o.v);
}

inline Vector2<float> constexpr Vector4f_a::xy() const {
    return Vector2<float>(v[0], v[1]);
}

inline Vector2<float> constexpr Vector4f_a::zw() const {
    return Vector2<float>(v[2], v[3]);
}

inline Vector3f_a constexpr Vector4f_a::xyz() const {
    return Vector3f_a(v);
}

template <typename T>
constexpr Vector4f_a::Vector4f_a(Vector4<T> const& a)
    : v{float(a[0]), float(a[1]), float(a[2]), float(a[3])} {}

inline float constexpr Vector4f_a::operator[](uint32_t i) const {
    return v[i];
}

inline float constexpr& Vector4f_a::operator[](uint32_t i) {
    return v[i];
}

static inline Vector4f_a constexpr operator+(Vector4f_a_p a, float s) {
    return Vector4f_a(a[0] + s, a[1] + s, a[2] + s, a[3] + s);
}

static inline constexpr Vector4f_a operator+(Vector4f_a_p a, Vector4f_a_p b) {
    return Vector4f_a(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);
}

static inline constexpr Vector4f_a operator-(Vector4f_a_p a, Vector4f_a_p b) {
    return Vector4f_a(a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]);
}

static inline constexpr Vector4f_a operator*(Vector4f_a_p a, Vector4f_a_p b) {
    return Vector4f_a(a[0] * b[0], a[1] * b[1], a[2] * b[2], a[3] * b[3]);
}

static inline constexpr Vector4f_a operator/(Vector4f_a_p a, float s) {
    float const is = 1.f / s;
    return Vector4f_a(is * a[0], is * a[1], is * a[2], is * a[3]);
}

static inline Vector4f_a constexpr operator-(Vector4f_a_p v) {
    return Vector4f_a(-v[0], -v[1], -v[2], -v[3]);
}

static inline constexpr Vector4f_a& operator+=(Vector4f_a& a, Vector4f_a_p b) {
    a[0] += b[0];
    a[1] += b[1];
    a[2] += b[2];
    a[3] += b[3];
    return a;
}

static inline constexpr Vector4f_a& operator-=(Vector4f_a& a, Vector4f_a_p b) {
    a[0] -= b[0];
    a[1] -= b[1];
    a[2] -= b[2];
    a[3] -= b[3];
    return a;
}

static inline constexpr bool operator==(Vector4f_a_p a, Vector4f_a_p b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static inline constexpr bool operator!=(Vector4f_a_p a, Vector4f_a_p b) {
    return a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3];
}

static inline constexpr Vector4f_a operator*(float s, Vector4f_a_p a) {
    return Vector4f_a(s * a[0], s * a[1], s * a[2], s * a[3]);
}

static inline float constexpr dot(Vector4f_a_p a, Vector4f_a_p b) {
    return (a[0] * b[0] + a[1] * b[1]) + (a[2] * b[2] + a[3] * b[3]);
}

static inline Vector4f_a constexpr lerp(Vector4f_a_p a, Vector4f_a_p b, float t) {
    float const u = 1.f - t;
    return u * a + t * b;
}

static inline Vector4f_a constexpr min(Vector4f_a_p a, float s) {
    return Vector4f_a(std::min(a[0], s), std::min(a[1], s), std::min(a[2], s), std::min(a[3], s));
}

static inline Vector4f_a constexpr min(Vector4f_a_p a, Vector4f_a_p b) {
    return Vector4f_a(std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]),
                      std::min(a[3], b[3]));
}

static inline float constexpr max_component(Vector4f_a_p v) {
    return std::max(std::max(std::max(v[0], v[1]), v[2]), v[3]);
}

static inline uint32_t constexpr index_min_component(Vector4f_a_p v) {
    float c = v[0];
    uint32_t i = 0;

    bool const yli = v[1] < v[i];

    c = yli ? v[1] : c;
    i = yli ? 1 : i;

    bool const zli = v[2] < v[i];

    c = zli ? v[2] : c;
    i = zli ? 2 : i;

    bool const wli = v[3] < v[i];

    c = wli ? v[3] : c;
    i = wli ? 3 : i;

    return i;
}

static inline float constexpr average(Vector4f_a_p c) {
    return (c[0] + c[1] + c[2] + c[3]) / 4.f;
}

static inline Vector4f_a abs(Vector4f_a_p v) {
    return Vector4f_a(std::abs(v[0]), std::abs(v[1]), std::abs(v[2]), std::abs(v[3]));
}

static inline bool all_finite(Vector4f_a_p v) {
    return std::isfinite(v[0]) && std::isfinite(v[1]) && std::isfinite(v[2]) && std::isfinite(v[3]);
}

static inline bool all_finite_and_positive(Vector4f_a_p v) {
    return std::isfinite(v[0]) && v[0] >= 0.f && std::isfinite(v[1]) && v[1] >= 0.f &&
           std::isfinite(v[2]) && v[2] >= 0.f && std::isfinite(v[2]) && v[3] >= 0.f;
}

//==============================================================================
// Aligned 4D int vector
//==============================================================================

inline constexpr Vector4i_a::Vector4i_a(int32_t x, int32_t y, int32_t z, int32_t w)
    : v{x, y, z, w} {}

inline constexpr Vector4i_a::Vector4i_a(Vector2<int32_t> xy, Vector2<int32_t> zw)
    : v{xy[0], xy[1], zw[0], zw[1]} {}

// inline constexpr Vector4i_a::Vector4i_a(int32_t s) : v{s, s, s, s} {}

inline constexpr Vector2<int32_t> Vector4i_a::xy() const {
    return Vector2<int32_t>(v[0], v[1]);
}

inline constexpr Vector2<int32_t> Vector4i_a::zw() const {
    return Vector2<int32_t>(v[2], v[3]);
}

inline constexpr int32_t Vector4i_a::operator[](uint32_t i) const {
    return v[i];
}

inline constexpr int32_t& Vector4i_a::operator[](uint32_t i) {
    return v[i];
}

static inline constexpr Vector4i_a operator+(Vector4i_a const& a, Vector4i_a const& b) {
    return Vector4i_a(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);
}

static inline constexpr Vector4i_a operator*(Vector4i_a const& a, Vector4i_a const& b) {
    return Vector4i_a(a[0] * b[0], a[1] * b[1], a[2] * b[2], a[3] * b[3]);
}

static inline constexpr Vector4i_a& operator+=(Vector4i_a& a, Vector4i_a const& b) {
    a[0] += b[0];
    a[1] += b[1];
    a[2] += b[2];
    a[3] += b[3];
    return a;
}

static inline constexpr Vector4i_a& operator-=(Vector4i_a& a, Vector4i_a const& b) {
    a[0] -= b[0];
    a[1] -= b[1];
    a[2] -= b[2];
    a[3] -= b[3];
    return a;
}

static inline constexpr bool operator!=(Vector4i_a const& a, Vector4i_a const& b) {
    return a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3];
}

}  // namespace math

#endif
