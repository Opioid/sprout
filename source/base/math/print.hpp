#pragma once

#include <ostream>

namespace math {

template<typename T> struct Vector2;
template<typename T> struct Vector3;
template<typename T> struct Matrix4x4;

struct alignas(16) Vector3f_a;

}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const math::Vector2<T>& v) {
	return stream << "[" << v.x << ", " << v.y << "]";
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const math::Vector3<T>& v) {
	return stream << "[" << v.x << ", " << v.y << ", " << v.z << "]";
}

inline std::ostream& operator<<(std::ostream& stream, const math::Vector3f_a& v) {
	return stream << "[" << v.x << ", " << v.y << ", " << v.z << "]";
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const math::Matrix4x4<T>& m) {
	return stream << "[" << m.m00 << ", " << m.m01 << ", " << m.m02 << ", " << m.m03 << "," << std::endl
				  << " " << m.m10 << ", " << m.m11 << ", " << m.m12 << ", " << m.m13 << "," << std::endl
				  << " " << m.m20 << ", " << m.m21 << ", " << m.m22 << ", " << m.m23 << "," << std::endl
				  << " " << m.m30 << ", " << m.m31 << ", " << m.m32 << ", " << m.m33 << "]";
}
