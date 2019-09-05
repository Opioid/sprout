#ifndef SU_CORE_SCENE_CONSTANTS_HPP
#define SU_CORE_SCENE_CONSTANTS_HPP

#include <cstdint>
#include <cstring>
#include "base/math/vector3.inl"

namespace scene {

float constexpr Ray_max_t = 2.e6f;

float constexpr Almost_ray_max_t = Ray_max_t - 0.08f;

float constexpr Almost_ray_max_t_minus_epsilon = Ray_max_t - 0.8f;

uint64_t constexpr Units_per_second = 705600000;

uint64_t constexpr Static_time = 0xffffffffffffffff;

static inline uint64_t constexpr time(double dtime) noexcept {
    return uint64_t(static_cast<double>(Units_per_second) * dtime);
}

// The following block implements the ray offset technique described in
// "A Fast and Robust Method for Avoiding Self-Intersection"

static inline float int_as_float(int32_t x) noexcept {
    float f;
    std::memcpy(&f, &x, sizeof(float));
    return f;
}

static inline int32_t float_as_int(float x) noexcept {
    int32_t i;
    std::memcpy(&i, &x, sizeof(int32_t));
    return i;
}

static float constexpr Origin      = 1.f / 32.f;
static float constexpr Float_scale = 1.f / 65536.f;
static float constexpr Int_scale   = 256.f;

static inline float3 offset_ray(float3 const& p, float3 const& n) noexcept {
    int3 const of_i(Int_scale * n);

    float3 const p_i(int_as_float(float_as_int(p[0]) + ((p[0] < 0.f) ? -of_i[0] : of_i[0])),
                     int_as_float(float_as_int(p[1]) + ((p[1] < 0.f) ? -of_i[1] : of_i[1])),
                     int_as_float(float_as_int(p[2]) + ((p[2] < 0.f) ? -of_i[2] : of_i[2])));

    return float3(std::abs(p[0]) < Origin ? p[0] + Float_scale * n[0] : p_i[0],
                  std::abs(p[1]) < Origin ? p[1] + Float_scale * n[1] : p_i[1],
                  std::abs(p[2]) < Origin ? p[2] + Float_scale * n[2] : p_i[2]);
}

static inline float offset_f(float t) noexcept {
    return t < Origin ? t + Float_scale
                      : int_as_float(float_as_int(t) + static_cast<int32_t>(Int_scale));
}

static inline float offset_b(float t) noexcept {
    return t < Origin ? t - Float_scale
                      : int_as_float(float_as_int(t) - static_cast<int32_t>(Int_scale));
}

}  // namespace scene

#endif
