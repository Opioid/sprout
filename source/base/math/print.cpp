#include "print.hpp"

std::ostream& operator<<(std::ostream& stream, const math::Vector3<uint8_t>& v) noexcept {
    return stream << "[" << static_cast<uint32_t>(v[0]) << ", " << static_cast<uint32_t>(v[1])
                  << ", " << static_cast<uint32_t>(v[2]) << "]";
}

std::ostream& operator<<(std::ostream& stream, const math::Vector3f_a& v) noexcept {
    return stream << "[" << v[0] << ", " << v[1] << ", " << v[2] << "]";
}

std::ostream& operator<<(std::ostream& stream, const math::Vector4f_a& v) noexcept {
    return stream << "[" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "]";
}

std::ostream& operator<<(std::ostream& stream, const math::Vector4i_a& v) noexcept {
    return stream << "[" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "]";
}

std::ostream& operator<<(std::ostream& stream, float3x3 const& m) noexcept {
    return stream << "[" << m.r[0][0] << ", " << m.r[0][1] << ", " << m.r[0][2] << ",\n"
                  << " " << m.r[1][0] << ", " << m.r[1][1] << ", " << m.r[1][2] << ",\n"
                  << " " << m.r[2][0] << ", " << m.r[2][1] << ", " << m.r[2][2] << "]";
}
