#ifndef SU_BASE_MATH_PRINT_HPP
#define SU_BASE_MATH_PRINT_HPP

#include <ostream>
#include "matrix4x4.hpp"
#include "vector4.inl"

template <typename T>
std::ostream& operator<<(std::ostream& stream, const math::Vector2<T>& v) {
    return stream << "[" << v[0] << ", " << v[1] << "]";
}

inline std::ostream& operator<<(std::ostream& stream, const math::Vector3<uint8_t>& v) {
    return stream << "[" << static_cast<uint32_t>(v[0]) << ", " << static_cast<uint32_t>(v[1])
                  << ", " << static_cast<uint32_t>(v[2]) << "]";
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, const math::Vector3<T>& v) {
    return stream << "[" << v[0] << ", " << v[1] << ", " << v[2] << "]";
}

inline std::ostream& operator<<(std::ostream& stream, const math::Vector3f_a& v) {
    return stream << "[" << v[0] << ", " << v[1] << ", " << v[2] << "]";
}

inline std::ostream& operator<<(std::ostream& stream, const math::Vector4f_a& v) {
    return stream << "[" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "]";
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, const math::Matrix3x3<T>& m) {
    return stream << "[" << m.m00 << ", " << m.m01 << ", " << m.m02 << ",\n"
                  << " " << m.m10 << ", " << m.m11 << ", " << m.m12 << ",\n"
                  << " " << m.m20 << ", " << m.m21 << ", " << m.m22 << "]";
}
/*
template<typename T>
std::ostream& operator<<(std::ostream& stream, const math::Matrix4x4<T>& m) {
        return stream << "[" << m.m00 << ", " << m.m01 << ", " << m.m02 << ", " << m.m03 << ",\n"
                                  << " " << m.m10 << ", " << m.m11 << ", " << m.m12 << ", " << m.m13
<< ",\n"
                                  << " " << m.m20 << ", " << m.m21 << ", " << m.m22 << ", " << m.m23
<< ",\n"
                                  << " " << m.m30 << ", " << m.m31 << ", " << m.m32 << ", " << m.m33
<< "]";
}
*/

#endif
