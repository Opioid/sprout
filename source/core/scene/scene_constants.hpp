#ifndef SU_CORE_SCENE_CONSTANTS_HPP
#define SU_CORE_SCENE_CONSTANTS_HPP

#include "base/math/vector3.inl"

#include <bit>
#include <cstdint>

namespace scene {

// offset_b(std::numeric_limits<float>::max());
inline float constexpr Ray_max_t = 3.4027715434167032e+38f;

// std::nextafter(Ray_max_t, 0.f);
inline float constexpr Almost_ray_max_t = 3.4027713405926072e+38f;

inline uint64_t constexpr Units_per_second = 705600000;

static inline uint64_t time(double dtime) {
    return uint64_t(std::llrint(double(Units_per_second) * dtime));
}

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

}  // namespace scene

#endif
