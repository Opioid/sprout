#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_ENCODING_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_ENCODING_INL

#include "base/math/vector3.inl"
#include "vertex_encoding.hpp"

namespace scene::shape::triangle {

// float constexpr F22 = float(1 << 22);
float constexpr FN = float((1 << 23) - 1);

inline void Norm23x2_Sign1x2::decode(packed_float3& v, float& s) const {
    uint32_t const a = (uint32_t(data[1]) & 0b01111111) << 16;
    uint32_t const x = a | uint32_t(data[0]);

    uint32_t const b = uint32_t(data[1]) >> 7;
    uint32_t const c = (uint32_t(data[2]) & 0b00111111'11111111) << 9;
    uint32_t const y = b | c;

    uint32_t const d = (uint32_t(data[2]) >> 14) & 0x1;
    uint32_t const e = (uint32_t(data[2]) >> 15) & 0x1;

    //	float const fx = float(x) / F22 - 1.f;
    //	float const fy = float(y) / F22 - 1.f;

    float const fx = (2.f * (float(x) / FN)) - 1.f;
    float const fy = (2.f * (float(y) / FN)) - 1.f;

    float const signs[2] = {1.f, -1.f};

    float const sz = signs[d];
    float const fz = sz * std::sqrt(1.f - (fx * fx + fy * fy));

    v[0] = fx;
    v[1] = fy;
    v[2] = fz;

    s = signs[e];
}

inline void Norm23x2_Sign1x2::encode(packed_float3 const& v, float s) {
    uint32_t const x = uint32_t((v[0] + 1.f) * (0.5f * FN));
    uint32_t const y = uint32_t((v[1] + 1.f) * (0.5f * FN));

    uint32_t const zs = v[3] < 0.f ? 1 : 0;
    uint32_t const is = s < 0.f ? 1 : 0;

    data[0] = static_cast<uint16_t>(x);
    data[1] = static_cast<uint16_t>((x >> 16) | (y << 7));
    data[2] = static_cast<uint16_t>((y >> 9) | (zs << 14) | (is << 15));
}

}  // namespace scene::shape::triangle

#endif
