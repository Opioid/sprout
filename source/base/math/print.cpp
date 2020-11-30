#include "print.hpp"

std::ostream& operator<<(std::ostream& stream, math::Vector3<uint8_t> v) {
    return stream << "[" << uint32_t(v[0]) << ", " << uint32_t(v[1]) << ", " << uint32_t(v[2])
                  << "]";
}

std::ostream& operator<<(std::ostream& stream, math::Vector3f_a_p v) {
    return stream << "[" << v[0] << ", " << v[1] << ", " << v[2] << "]";
}

std::ostream& operator<<(std::ostream& stream, math::Vector4f_a_p v) {
    return stream << "[" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "]";
}

std::ostream& operator<<(std::ostream& stream, math::Vector4i_a v) {
    return stream << "[" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "]";
}

std::ostream& operator<<(std::ostream& stream, float3x3 const& m) {
    return stream << "[" << m.r[0][0] << ", " << m.r[0][1] << ", " << m.r[0][2] << ",\n"
                  << " " << m.r[1][0] << ", " << m.r[1][1] << ", " << m.r[1][2] << ",\n"
                  << " " << m.r[2][0] << ", " << m.r[2][1] << ", " << m.r[2][2] << "]";
}

std::ostream& operator<<(std::ostream& stream, float4x4 const& m) {
    return stream << "[" << m.r[0][0] << ", " << m.r[0][1] << ", " << m.r[0][2] << ", " << m.r[0][3]
                  << ",\n"
                  << " " << m.r[1][0] << ", " << m.r[1][1] << ", " << m.r[1][2] << ", " << m.r[1][3]
                  << ",\n"
                  << " " << m.r[2][0] << ", " << m.r[2][1] << ", " << m.r[2][2] << ", " << m.r[2][3]
                  << ",\n"
                  << " " << m.r[3][0] << ", " << m.r[3][1] << ", " << m.r[3][2] << ", " << m.r[3][3]
                  << "]";
}
