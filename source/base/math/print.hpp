#pragma once

#include "vector3.hpp"
#include <ostream>

template<typename T>
std::ostream& operator<<(std::ostream& stream, const math::Vector3<T>& v) {
	return stream << "[" << v.x << ", " << v.y << ", " << v.z << "]";
}
