#ifndef SU_CORE_SCENE_RAY_OFFSET_INL
#define SU_CORE_SCENE_RAY_OFFSET_INL

#include "base/math/vector3.inl"

#include <bit>
#include <cstdint>

namespace scene {

// The following block implements the ray offset technique described in
// "A Fast and Robust Method for Avoiding Self-Intersection"

inline float constexpr Origin      = 1.f / 32.f;
inline float constexpr Float_scale = 1.f / 65536.f;
inline float constexpr Int_scale   = 256.f;

static inline float3 offset_ray(float3_p p, float3_p n) {
    int3 const of_i(Int_scale * n);

    float3 const p_i(
        std::bit_cast<float>(std::bit_cast<int32_t>(p[0]) + ((p[0] < 0.f) ? -of_i[0] : of_i[0])),
        std::bit_cast<float>(std::bit_cast<int32_t>(p[1]) + ((p[1] < 0.f) ? -of_i[1] : of_i[1])),
        std::bit_cast<float>(std::bit_cast<int32_t>(p[2]) + ((p[2] < 0.f) ? -of_i[2] : of_i[2])));

    return float3(std::abs(p[0]) < Origin ? std::fma(Float_scale, n[0], p[0]) : p_i[0],
                  std::abs(p[1]) < Origin ? std::fma(Float_scale, n[1], p[1]) : p_i[1],
                  std::abs(p[2]) < Origin ? std::fma(Float_scale, n[2], p[2]) : p_i[2]);
}

static inline float offset_f(float t) {
    return t < Origin ? t + Float_scale
                      : std::bit_cast<float>(std::bit_cast<int32_t>(t) + int32_t(Int_scale));
}

static inline float offset_b(float t) {
    return t < Origin ? t - Float_scale
                      : std::bit_cast<float>(std::bit_cast<int32_t>(t) - int32_t(Int_scale));
}

static inline float offset_b(float3_p p, float3_p n, float t) {
    float const s = max_component(abs(p * n));
    return t - (offset_f(s) - s);
}

}  // namespace scene

#endif
