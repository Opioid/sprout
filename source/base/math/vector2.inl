#pragma once

#include "vector2.hpp"
#include <algorithm>
#include <cmath>

namespace math {

template<typename T>
Vector2<T>::Vector2() {}

template<typename T>
Vector2<T>::Vector2(T s) : v{s, s} {}

template<typename T>
Vector2<T>::Vector2(T x, T y) : v{x, y} {}

template<typename T>
template<typename U>
Vector2<T>::Vector2(Vector2<U> v) : v{static_cast<T>(v[0]), static_cast<T>(v[1])} {}

template<typename T>
Vector2<T> Vector2<T>::yx() const {
	return Vector2(v[1], v[0]);
}

template<typename T>
T Vector2<T>::operator[](uint32_t i) const{
	return v[i];
}

template<typename T>
T& Vector2<T>::operator[](uint32_t i) {
	return v[i];
}

template<typename T>
Vector2<T> Vector2<T>::operator+(Vector2 a) const {
	return Vector2(v[0] + a[0], v[1] + a[1]);
}

template<typename T>
Vector2<T> Vector2<T>::operator-(Vector2 a) const {
	return Vector2(v[0] - a[0], v[1] - a[1]);
}

template<typename T>
Vector2<T> Vector2<T>::operator*(Vector2 a) const {
	return Vector2(v[0] * a[0], v[1] * a[1]);
}

template<typename T>
Vector2<T> Vector2<T>::operator/(T s) const {
	return Vector2(v[0] / s, v[1] / s);
}

template<typename T>
Vector2<T> Vector2<T>::operator/(Vector2 a) const {
	return Vector2(v[0] / a[0], v[1] / a[1]);
}

template<typename T>
Vector2<T>& Vector2<T>::operator+=(Vector2 a) {
	v[0] += a[0];
	v[1] += a[1];
	return *this;
}

template<typename T>
Vector2<T>& Vector2<T>::operator*=(Vector2 a) {
	v[0] *= a[0];
	v[1] *= a[1];
	return *this;
}

template<typename T>
Vector2<T>& Vector2<T>::operator/=(T s) {
	v[0] /= s;
	v[1] /= s;
	return *this;
}

template<typename T>
bool Vector2<T>::operator==(Vector2 a) const {
	return v[0] == a[0] && v[1] == a[1];
}

template<typename T>
bool Vector2<T>::operator!=(Vector2 a) const {
	return v[0] != a[0] || v[1] != a[1];
}

template<typename T>
const Vector2<T> Vector2<T>::identity(T(0), T(0));

template<typename T>
Vector2<T> operator*(T s, Vector2<T> v) {
	return Vector2<T>(s * v[0], s * v[1]);
}

template<typename T>
Vector2<T> operator/(T s, Vector2<T> v) {
	return Vector2<T>(s / v[0], s / v[1]);
}

template<typename T>
T dot(Vector2<T> a, Vector2<T> b) {
	return a[0] * b[0] + a[1] * b[1];
}

template<typename T>
T length(Vector2<T> v)  {
	return std::sqrt(dot(v, v));
}

template<typename T>
T squared_length(Vector2<T> v) {
	return dot(v, v);
}

template<typename T>
Vector2<T> normalized(Vector2<T> v) {
	return v / length(v);
}

template<typename T>
Vector2<T> reciprocal(Vector2<T> v) {
	return Vector2<T>(T(1) / v[0], T(1) / v[1]);
}

template<typename T>
T distance(Vector2<T> a, Vector2<T> b) {
	return length(a - b);
}

template<typename T>
Vector2<T> saturate(Vector2<T> v) {
	return Vector2<T>(std::min(std::max(v[0], T(0)), T(1)),
					  std::min(std::max(v[1], T(0)), T(1)));
}

template<typename T>
Vector2<T> lerp(Vector2<T> a, Vector2<T> b, T t) {
	T u = T(1) - t;
	return u * a + t * b;
}

template<typename T>
Vector2<T> round(Vector2<T> v){
	return Vector2<T>(std::floor(v[0] >= T(0) ? v[0] + T(0.5) : v[0] - T(0.5)),
					  std::floor(v[1] >= T(0) ? v[1] + T(0.5) : v[1] - T(0.5)));
}

template<typename T>
Vector2<T> min(Vector2<T> a, Vector2<T> b) {
	return Vector2<T>(a[0] < b[0] ? a[0] : b[0], a[1] < b[1] ? a[1] : b[1]);
}

template<typename T>
Vector2<T> max(Vector2<T> a, Vector2<T> b) {
	return Vector2<T>(a[0] > b[0] ? a[0] : b[0], a[1] > b[1] ? a[1] : b[1]);
}

template<typename T>
bool any_nan(const Vector2<T> v) {
	return std::isnan(v[0]) || std::isnan(v[1]);
}

template<typename T>
bool any_inf(const Vector2<T> v) {
	return std::isinf(v[0]) || std::isinf(v[1]);
}

template<typename T>
bool all_finite(const Vector2<T> v) {
	return std::isfinite(v[0]) && std::isfinite(v[1]);
}

}
