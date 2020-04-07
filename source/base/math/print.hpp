#ifndef SU_BASE_MATH_PRINT_HPP
#define SU_BASE_MATH_PRINT_HPP

#include "matrix.hpp"
#include "matrix3x3.hpp"
#include "matrix4x4.hpp"
#include "vector4.inl"

#include <ostream>

template <typename T>
std::ostream& operator<<(std::ostream& stream, const math::Vector2<T>& v) {
    return stream << "[" << v[0] << ", " << v[1] << "]";
}

std::ostream& operator<<(std::ostream& stream, const math::Vector3<uint8_t>& v);

template <typename T>
std::ostream& operator<<(std::ostream& stream, const math::Vector3<T>& v) {
    return stream << "[" << v[0] << ", " << v[1] << ", " << v[2] << "]";
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, const math::Vector4<T>& v) {
    return stream << "[" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "]";
}

std::ostream& operator<<(std::ostream& stream, const math::Vector3f_a& v);

std::ostream& operator<<(std::ostream& stream, const math::Vector4f_a& v);

std::ostream& operator<<(std::ostream& stream, const math::Vector4i_a& v);

template <typename T>
std::ostream& operator<<(std::ostream& stream, const math::Matrix3x3<T>& m) {
    return stream << "[" << m.m00 << ", " << m.m01 << ", " << m.m02 << ",\n"
                  << " " << m.m10 << ", " << m.m11 << ", " << m.m12 << ",\n"
                  << " " << m.m20 << ", " << m.m21 << ", " << m.m22 << "]";
}

std::ostream& operator<<(std::ostream& stream, float3x3 const& m);

std::ostream& operator<<(std::ostream& stream, float4x4 const& m);

#endif
