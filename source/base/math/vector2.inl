#pragma once

#include "vector2.hpp"
#include <algorithm>
#include <cmath>

namespace math {

template<typename T>
constexpr Vector2<T>::Vector2(T s) : v{s, s} {}

template<typename T>
constexpr Vector2<T>::Vector2(T x, T y) : v{x, y} {}

template<typename T>
template<typename U>
constexpr Vector2<T>::Vector2(Vector2<U> v) : v{T(v[0]), T(v[1])} {}

template<typename T>
constexpr Vector2<T> Vector2<T>::yx() const {
	return Vector2<T>(v[1], v[0]);
}

template<typename T>
constexpr T Vector2<T>::operator[](uint32_t i) const {
	return v[i];
}

template<typename T>
constexpr T& Vector2<T>::operator[](uint32_t i) {
	return v[i];
}

template<typename T>
constexpr Vector2<T> Vector2<T>::identity() {
	return Vector2<T>(T(0), T(0));
}

template<typename T>
static constexpr Vector2<T> operator+(Vector2<T> a, Vector2<T> b) {
	return Vector2<T>(a[0] + b[0], a[1] + b[1]);
}

template<typename T>
static constexpr Vector2<T> operator-(Vector2<T> a, Vector2<T> b) {
	return Vector2<T>(a[0] - b[0], a[1] - b[1]);
}

template<typename T>
static constexpr Vector2<T> operator*(Vector2<T> a, Vector2<T> b) {
	return Vector2<T>(a[0] * b[0], a[1] * b[1]);
}

template<typename T>
static constexpr Vector2<T> operator/(Vector2<T> a, T s) {
	return Vector2<T>(a[0] / s, a[1] / s);
}

template<typename T>
static constexpr Vector2<T> operator/(Vector2<T> a, Vector2<T> b) {
	return Vector2<T>(a[0] / b[0], a[1] / b[1]);
}

template<typename T>
static constexpr Vector2<T>& operator+=(Vector2<T>& a, Vector2<T> b) {
	a[0] += b[0];
	a[1] += b[1];
	return a;
}

template<typename T>
static constexpr Vector2<T>& operator*=(Vector2<T>& a, float s) {
	a[0] *= s;
	a[1] *= s;
	return a;
}

template<typename T>
static constexpr Vector2<T>& operator*=(Vector2<T>& a, Vector2<T> b) {
	a[0] *= b[0];
	a[1] *= b[1];
	return a;
}

template<typename T>
static constexpr Vector2<T>& operator/=(Vector2<T>& a, T s) {
	a[0] /= s;
	a[1] /= s;
	return a;
}

template<typename T>
static constexpr bool operator==(Vector2<T> a, Vector2<T> b) {
	return a[0] == b[0] && a[1] == b[1];
}

template<typename T>
static constexpr bool operator!=(Vector2<T> a, Vector2<T> b) {
	return a[0] != b[0] || a[1] != b[1];
}

template<typename T>
static constexpr Vector2<T> operator*(T s, Vector2<T> v) {
	return Vector2<T>(s * v[0], s * v[1]);
}

template<typename T>
static constexpr Vector2<T> operator/(T s, Vector2<T> v) {
	return Vector2<T>(s / v[0], s / v[1]);
}

template<typename T>
static constexpr T dot(Vector2<T> a, Vector2<T> b) {
	return a[0] * b[0] + a[1] * b[1];
}

template<typename T>
static constexpr T length(Vector2<T> v)  {
	return std::sqrt(dot(v, v));
}

template<typename T>
static constexpr T squared_length(Vector2<T> v) {
	return dot(v, v);
}

template<typename T>
static constexpr Vector2<T> normalize(Vector2<T> v) {
	return v / length(v);
}

template<typename T>
static constexpr Vector2<T> reciprocal(Vector2<T> v) {
	return Vector2<T>(T(1) / v[0], T(1) / v[1]);
}

template<typename T>
static constexpr T distance(Vector2<T> a, Vector2<T> b) {
	return length(a - b);
}

template<typename T>
static constexpr Vector2<T> saturate(Vector2<T> v) {
	return Vector2<T>(std::min(std::max(v[0], T(0)), T(1)),
					  std::min(std::max(v[1], T(0)), T(1)));
}

template<typename T>
static constexpr Vector2<T> lerp(Vector2<T> a, Vector2<T> b, T t) {
	const T u = T(1) - t;
	return u * a + t * b;
}

template<typename T>
static constexpr Vector2<T> round(Vector2<T> v){
	return Vector2<T>(std::floor(v[0] >= T(0) ? v[0] + T(0.5) : v[0] - T(0.5)),
					  std::floor(v[1] >= T(0) ? v[1] + T(0.5) : v[1] - T(0.5)));
}

template<typename T>
static constexpr Vector2<T> min(Vector2<T> a, Vector2<T> b) {
	return Vector2<T>(a[0] < b[0] ? a[0] : b[0], a[1] < b[1] ? a[1] : b[1]);
}

template<typename T>
static constexpr Vector2<T> max(Vector2<T> a, Vector2<T> b) {
	return Vector2<T>(a[0] > b[0] ? a[0] : b[0], a[1] > b[1] ? a[1] : b[1]);
}

template<typename T>
static bool any_nan(const Vector2<T> v) {
	return std::isnan(v[0]) || std::isnan(v[1]);
}

template<typename T>
static bool any_inf(const Vector2<T> v) {
	return std::isinf(v[0]) || std::isinf(v[1]);
}

template<typename T>
static bool all_finite(const Vector2<T> v) {
	return std::isfinite(v[0]) && std::isfinite(v[1]);
}

}
