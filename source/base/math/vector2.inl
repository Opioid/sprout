#pragma once

#include "vector2.hpp"
#include <cmath>

namespace math {

template<typename T>
Vector2<T>::Vector2() {}

template<typename T>
Vector2<T>::Vector2(T x, T y) : x(x), y(y) {}

template<typename T>
template<typename U>
Vector2<T>::Vector2(Vector2<U> v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)) {}

template<typename T>
Vector2<T> Vector2<T>::yx() const {
	return Vector2(y, x);
}

template<typename T>
Vector2<T> Vector2<T>::operator+(Vector2 v) const {
	return Vector2(x + v.x, y + v.y);
}

template<typename T>
Vector2<T> Vector2<T>::operator-(Vector2 v) const {
	return Vector2(x - v.x, y - v.y);
}

template<typename T>
Vector2<T> Vector2<T>::operator*(Vector2 v) const {
	return Vector2(x * v.x, y * v.y);
}

template<typename T>
Vector2<T> Vector2<T>::operator/(T s) const {
	return Vector2(x / s, y / s);
}

template<typename T>
Vector2<T>& Vector2<T>::operator+=(Vector2 v) {
	x += v.x;
	y += v.y;
	return *this;
}

template<typename T>
Vector2<T>& Vector2<T>::operator/=(T s) {
	x /= s;
	y /= s;
	return *this;
}

template<typename T>
bool Vector2<T>::operator==(Vector2 v) const {
	return x == v.x && y == v.y;
}

template<typename T>
bool Vector2<T>::operator!=( Vector2 v) const {
	return x != v.x || y != v.y;
}

template<typename T>
const Vector2<T> Vector2<T>::identity(T(0), T(0));

template<typename T>
Vector2<T> operator*(T s, Vector2<T> v) {
	return Vector2<T>(s * v.x, s * v.y);
}

template<typename T>
Vector2<T> operator/(T s, Vector2<T> v) {
	return Vector2<T>(s / v.x, s / v.y);
}

template<typename T>
T dot(Vector2<T> a, Vector2<T> b) {
	return a.x * b.x + a.y * b.y;
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
	return Vector2<T>(T(1) / v.x, T(1) / v.y);
}

template<typename T>
Vector2<T> lerp(Vector2<T> a, Vector2<T> b, T t) {
	T u = T(1) - t;
	return u * a + t * b;
}

template<typename T>
Vector2<T> round(Vector2<T> v){
	return Vector2<T>(std::floor(v.x >= T(0) ? v.x + T(0.5) : v.x - T(0.5)),
					  std::floor(v.y >= T(0) ? v.y + T(0.5) : v.y - T(0.5)));
}

template<typename T>
Vector2<T> min(Vector2<T> a, Vector2<T> b) {
	return Vector2<T>(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y);
}

template<typename T>
Vector2<T> max(Vector2<T> a, Vector2<T> b) {
	return Vector2<T>(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y);
}

template<typename T>
bool contains_nan(const Vector2<T>& v) {
	return std::isnan(v.x) || std::isnan(v.y);
}

template<typename T>
bool contains_inf(const Vector2<T>& v) {
	return std::isinf(v.x) || std::isinf(v.y);
}

}
