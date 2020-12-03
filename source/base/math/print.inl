#ifndef SU_BASE_MATH_PRINT_INL
#define SU_BASE_MATH_PRINT_INL

#include "print.hpp"
#include "vector4.inl"

#include <ostream>

template <typename T>
std::ostream& operator<<(std::ostream& stream, math::Vector2<T> v) {
    return stream << "[" << v[0] << ", " << v[1] << "]";
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, math::Vector3<T> v) {
    return stream << "[" << v[0] << ", " << v[1] << ", " << v[2] << "]";
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, math::Vector4<T> v) {
    return stream << "[" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "]";
}

#endif
