#ifndef SU_BASE_MATH_VECTOR4_INL
#define SU_BASE_MATH_VECTOR4_INL

#include "vector4.hpp"
#include "vector3.inl"

namespace math {

//==============================================================================
// Generic 4D vector
//==============================================================================

template<typename T>
constexpr Vector4<T>::Vector4(T x, T y, T z, T w) : v{x, y, z, w} {}

template<typename T>
constexpr Vector4<T>::Vector4(T s) : v{s, s, s, s} {}

template<typename T>
constexpr Vector4<T>::Vector4(Vector2<T> xy, T z, T w) : v{xy[0], xy[1], z, w} {}

template<typename T>
constexpr Vector4<T>::Vector4(const Vector3<T>& xyz, T w) : v{xyz[0], xyz[1], xyz[2], w} {}

template<typename T>
constexpr Vector3<T> Vector4<T>::xyz() const {
	return Vector3<T>(v);
}

template<typename T>
constexpr T Vector4<T>::operator[](uint32_t i) const{
	return v[i];
}

template<typename T>
constexpr T& Vector4<T>::operator[](uint32_t i) {
	return v[i];
}

template<typename T>
constexpr Vector4<T> Vector4<T>::operator+(const Vector4& v) const {
	return Vector4(v[0] + v[0], v[1] + v[1], v[2] + v[2], v[3] + v[3]);
}

template<typename T>
constexpr Vector4<T> Vector4<T>::operator*(const Vector4& v) const {
	return Vector4(v[0] * v[0], v[1] * v[1], v[2] * v[2], v[3] * v[3]);
}

template<typename T>
constexpr Vector4<T> Vector4<T>::operator/(T s) const {
	T is = T(1) / s;
	return Vector4(is * v[0], is * v[1], is * v[2], is * v[2]);
}

template<typename T>
constexpr Vector4<T>& Vector4<T>::operator+=(const Vector4& a) {
	v[0] += a[0]; v[1] += a[1]; v[2] += a[2]; v[3] += a[3];
	return *this;
}

template<typename T>
constexpr Vector4<T>& Vector4<T>::operator-=(const Vector4& a) {
	v[0] -= a[0]; v[1] -= a[1]; v[2] -= a[2]; v[3] -= a[3];
	return *this;
}

template<typename T>
static constexpr bool operator==(const Vector4<T>& a, const Vector4<T>& b) {
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

template<typename T>
static constexpr bool operator!=(const Vector4<T>& a, const Vector4<T>& b) {
	return a[0] != b[0] && a[1] != b[1] && a[2] != b[2] && a[3] != b[3];
}

template<typename T>
constexpr Vector4<T> Vector4<T>::identity() {
	return Vector4(T(0), T(0), T(0), T(0));
}

template<typename T>
constexpr Vector4<T> operator*(T s, const Vector4<T> &v) {
	return Vector4<T>(s * v[0], s * v[1], s * v[2], s * v[3]);
}

template<typename T>
constexpr T dot(const Vector4<T>& a, const Vector4<T>& b) {
	return (a[0] * b[0] + a[1] * b[1]) + (a[2] * b[2] + a[3] * b[3]);
}

//==============================================================================
// Aligned 4D float vector
//==============================================================================

inline constexpr Vector4f_a::Vector4f_a(float x, float y, float z, float w) :
	v{x, y, z, w} {}

inline constexpr Vector4f_a::Vector4f_a(float s) : v{s, s, s, s} {}

inline constexpr Vector4f_a::Vector4f_a(const Vector2<float> xy, float z, float w) :
	v{xy[0], xy[1], z, w} {}

inline constexpr Vector4f_a::Vector4f_a(FVector3f_a xyz, float w) :
	v{xyz[0], xyz[1], xyz[2], w} {}

inline constexpr Vector4f_a::Vector4f_a(const Vector3<float>& xyz, float w) :
	v{xyz[0], xyz[1], xyz[2], w} {}

inline constexpr Vector3f_a Vector4f_a::xyz() const {
	return Vector3f_a(v);
}

inline constexpr float Vector4f_a::operator[](uint32_t i) const {
	return v[i];
}

inline constexpr float& Vector4f_a::operator[](uint32_t i) {
	return v[i];
}

inline constexpr Vector4f_a Vector4f_a::identity() {
	return Vector4f_a(0.f, 0.f, 0.f, 0.f);
}

static inline constexpr Vector4f_a operator+(FVector4f_a a, FVector4f_a b) {
	return Vector4f_a(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);
}

static inline constexpr Vector4f_a operator*(FVector4f_a a, FVector4f_a b) {
	return Vector4f_a(a[0] * b[0], a[1] * b[1], a[2] * b[2], a[3] * b[3]);
}

static inline constexpr Vector4f_a operator/(FVector4f_a a, float s) {
	float const is = 1.f / s;
	return Vector4f_a(is * a[0], is * a[1], is * a[2], is * a[3]);
}

static inline constexpr Vector4f_a& operator+=(Vector4f_a& a, FVector4f_a b) {
	a[0] += b[0]; a[1] += b[1]; a[2] += b[2]; a[3] += b[3];
	return a;
}

static inline constexpr Vector4f_a& operator-=(Vector4f_a& a, FVector4f_a b) {
	a[0] -= b[0]; a[1] -= b[1]; a[2] -= b[2]; a[3] -= b[3];
	return a;
}

static inline constexpr bool operator==(FVector4f_a a, FVector4f_a b) {
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static inline constexpr bool operator!=(FVector4f_a a, FVector4f_a b) {
	return a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3];
}

static inline constexpr Vector4f_a operator*(float s, FVector4f_a a) {
	return Vector4f_a(s * a[0], s * a[1], s * a[2], s * a[3]);
}

static inline constexpr float dot(FVector4f_a a, FVector4f_a b) {
	return (a[0] * b[0] + a[1] * b[1]) + (a[2] * b[2] + a[3] * b[3]);
}

//==============================================================================
// Aligned 4D int vector
//==============================================================================

inline constexpr Vector4i_a::Vector4i_a(int32_t x, int32_t y, int32_t z, int32_t w) :
		v{x, y, z, w} {}

inline constexpr Vector4i_a::Vector4i_a(const Vector2<int32_t> xy, const Vector2<int32_t> zw) :
	v{xy[0], xy[1], zw[0], zw[1]} {}

inline constexpr Vector4i_a::Vector4i_a(int32_t s) : v{s, s, s, s} {}

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

static inline constexpr Vector4i_a operator+(const Vector4i_a& a, const Vector4i_a& b) {
	return Vector4i_a(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);
}

static inline constexpr Vector4i_a operator*(const Vector4i_a& a, const Vector4i_a& b) {
	return Vector4i_a(a[0] * b[0], a[1] * b[1], a[2] * b[2], a[3] * b[3]);
}

static inline constexpr Vector4i_a& operator+=(Vector4i_a& a, const Vector4i_a& b) {
	a[0] += b[0]; a[1] += b[1]; a[2] += b[2]; a[3] += b[3];
	return a;
}

static inline constexpr Vector4i_a& operator-=(Vector4i_a& a, const Vector4i_a& b) {
	a[0] -= b[0]; a[1] -= b[1]; a[2] -= b[2]; a[3] -= b[3];
	return a;
}

}

#endif
