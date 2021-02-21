#ifndef SU_CORE_RENDERING_SENSOR_CLAMP_INL
#define SU_CORE_RENDERING_SENSOR_CLAMP_INL

#include "base/math/vector4.inl"
#include "clamp.hpp"

namespace rendering::sensor::clamp {

inline float4 Identity::clamp(float4_p color) const {
    return color;
}

inline Clamp::Clamp(float3_p max) : max_(max) {}

inline float4 Clamp::clamp(float4_p color) const {
    return float4(std::min(color[0], max_[0]), std::min(color[1], max_[1]),
                  std::min(color[2], max_[2]), color[3]);
}

inline Luminance::Luminance(float max) : max_(max) {}

inline float4 Luminance::clamp(float4_p color) const {
    float3 const rgb = color.xyz();

    float const mc = max_component(rgb);

    if (mc > max_) {
        float const r = max_ / mc;

        return float4(r * rgb, color[3]);
    }

    return color;
}

}  // namespace rendering::sensor::clamp

#endif
