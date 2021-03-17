#ifndef SU_CORE_RENDERING_INTEGRATOR_HELPER_HPP
#define SU_CORE_RENDERING_INTEGRATOR_HELPER_HPP

#include "base/math/vector4.inl"

namespace rendering {

enum class Event { Absorb, Scatter, Pass, Abort };

static inline float attenuation(float distance, float c) {
    return ::exp(-distance * c);
}

static inline float3 attenuation(float distance, float3_p c) {
    return exp(-distance * c);
}

static inline float4 compose_alpha(float3_p radiance, float3_p throughput, bool transparent) {
    if (transparent) {
        float const alpha = std::max(1.f - average(throughput), 0.f);

        return float4(radiance /*+ alpha * throughput*/, alpha);
    }

    return float4(radiance, 1.f);
}

static inline float balance_heuristic(float f_pdf, float g_pdf) {
    return f_pdf / (f_pdf + g_pdf);
}

static inline float power_heuristic(float f_pdf, float g_pdf) {
    float const f2 = f_pdf * f_pdf;
    return f2 / (f2 + g_pdf * g_pdf);
}

// == power_heuristic(a, b) / a
static inline float predivided_power_heuristic(float f_pdf, float g_pdf) {
    return f_pdf / (f_pdf * f_pdf + g_pdf * g_pdf);
}

static inline bool russian_roulette(float3& throughput, float r) {
    float const continuation_probability = max_component(throughput);

    if (r > continuation_probability) {
        return true;
    }

    throughput /= continuation_probability;

    return false;
}

}  // namespace rendering

#endif
