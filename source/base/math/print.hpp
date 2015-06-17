#pragma once

#include <ostream>

template<typename T> struct Vector2;
template<typename T> struct Vector3;

template<typename T>
std::ostream& operator<<(std::ostream& stream, const math::Vector2<T>& v) {
	return stream << "[" << v.x << ", " << v.y << "]";
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const math::Vector3<T>& v) {
	return stream << "[" << v.x << ", " << v.y << ", " << v.z << "]";
}
