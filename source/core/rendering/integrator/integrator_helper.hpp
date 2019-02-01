#ifndef SU_CORE_RENDERING_INTEGRATOR_HELPER_HPP
#define SU_CORE_RENDERING_INTEGRATOR_HELPER_HPP

#include <cmath>
#include "base/math/vector3.inl"

namespace rendering {

enum class Event { Absorb, Scatter, Pass, Undefined };

static inline float attenuation(float distance, float c) noexcept {
    return math::exp(-distance * c);
}

static inline float3 attenuation(float distance, float3 const& c) noexcept {
    return math::exp(-distance * c);
}

static inline float3 attenuation(float3 const& start, float3 const& end, float3 const& c) noexcept {
    return attenuation(math::distance(start, end), c);
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

}  // namespace rendering

#endif
