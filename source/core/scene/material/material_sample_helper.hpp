#ifndef SU_CORE_SCENE_MATERIAL_SAMPLE_HELPER_HPP
#define SU_CORE_SCENE_MATERIAL_SAMPLE_HELPER_HPP

#include "base/math/vector3.inl"

namespace scene::material {

static inline float abs_dot(float3_p a, float3_p b) {
    return std::abs(dot(a, b));
}

inline float constexpr Dot_min = 0.00001f;

static inline float clamp(float x) {
    return std::clamp(x, Dot_min, 1.f);
}

static inline float clamp_abs(float x) {
    return std::clamp(std::abs(x), Dot_min, 1.f);
}

static inline float clamp_dot(float3_p a, float3_p b) {
    return std::clamp(dot(a, b), Dot_min, 1.f);
}

static inline float clamp_abs_dot(float3_p a, float3_p b) {
    return std::clamp(std::abs(dot(a, b)), Dot_min, 1.f);
}

static inline bool refract(float3_p n, float3_p v, float eta, float3& vr) {
    float const n_dot_wo = std::min(std::abs(dot(n, v)), 1.f);
    float const sint2    = (eta * eta) * (1.f - n_dot_wo * n_dot_wo);

    if (sint2 >= 1.f) {
        return false;
    }

    float const n_dot_t = std::sqrt(1.f - sint2);

    vr = -normalize((eta * n_dot_wo - n_dot_t) * n - eta * v);

    return true;
}

}  // namespace scene::material

#endif
