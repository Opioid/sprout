#ifndef SU_CORE_SCENE_MATERIAL_SAMPLE_HELPER_HPP
#define SU_CORE_SCENE_MATERIAL_SAMPLE_HELPER_HPP

#include "base/math/vector3.inl"

namespace scene::material {

static inline float abs_dot(float3 const& a, float3 const& b) noexcept {
    return std::abs(math::dot(a, b));
}

constexpr float Dot_min = 0.00001f;

static inline float clamp(float x) noexcept {
    return std::clamp(x, Dot_min, 1.f);
}

static inline float clamp_dot(float3 const& a, float3 const& b) noexcept {
    return std::clamp(math::dot(a, b), Dot_min, 1.f);
}

static inline float clamp_reverse_dot(float3 const& a, float3 const& b) noexcept {
    return std::clamp(-math::dot(a, b), Dot_min, 1.f);
}

static inline float clamp_abs_dot(float3 const& a, float3 const& b) noexcept {
    return std::clamp(std::abs(math::dot(a, b)), Dot_min, 1.f);
}

static inline bool refract(float3 const& n, float3 const& v, float eta, float3& vr) noexcept {
    float const n_dot_wo = std::min(std::abs(math::dot(n, v)), 1.f);
    float const sint2    = (eta * eta) * (1.f - n_dot_wo * n_dot_wo);

    if (sint2 >= 1.f) {
        return false;
    }

    float const n_dot_t = std::sqrt(1.f - sint2);

    vr = -math::normalize((eta * n_dot_wo - n_dot_t) * n - eta * v);

    return true;
}

}  // namespace scene::material

#endif
