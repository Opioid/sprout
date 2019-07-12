#ifndef SU_CORE_RENDERING_INTEGRATOR_HELPER_HPP
#define SU_CORE_RENDERING_INTEGRATOR_HELPER_HPP

#include <cmath>
#include "base/math/vector4.inl"

namespace rendering {

enum class Event { Absorb, Scatter, Pass, Abort };

static inline float attenuation(float distance, float c) noexcept {
    return math::exp(-distance * c);
}

static inline float3 attenuation(float distance, float3 const& c) noexcept {
    return math::exp(-distance * c);
}

static inline float3 attenuation(float3 const& start, float3 const& end, float3 const& c) noexcept {
    return attenuation(math::distance(start, end), c);
}

static inline float4 compose_alpha(float3 const& radiance, float3 const& throughput,
                                   bool transparent) noexcept {
    if (transparent) {
        float const alpha = std::max(1.f - average(throughput), 0.f);

        return float4(radiance /*+ alpha * throughput*/, alpha);
    }

    return float4(radiance, 1.f);
}

static inline float balance_heuristic(float f_pdf, float g_pdf) noexcept {
    return f_pdf / (f_pdf + g_pdf);
}

static inline float power_heuristic(float f_pdf, float g_pdf) noexcept {
    float const f2 = f_pdf * f_pdf;
    return f2 / (f2 + g_pdf * g_pdf);
}

static inline bool russian_roulette(float3& throughput, float r) noexcept {
    float const continuation_probability = max_component(throughput);

    if (r > continuation_probability) {
        return true;
    }

    throughput /= continuation_probability;

    return false;
}

static inline float non_symmetry_compensation(float3 const& wi, float3 const& wo,
                                              float3 const& geo_n, float3 const& n) noexcept {
    // Veach's compensation for "Non-symmetry due to shading normals".
    // See e.g. CorrectShadingNormal() at:
    // https://github.com/mmp/pbrt-v3/blob/master/src/integrators/bdpt.cpp#L55

    float const numer = std::abs(dot(wo, geo_n) * dot(wi, n));
    float const denom = std::abs(dot(wo, n) * dot(wi, geo_n));

    return (numer / std::max(denom, 0.01f));
}

}  // namespace rendering

#endif
