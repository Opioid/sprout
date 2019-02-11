#include "substitute_translucent_sample.hpp"
#include "base/math/math.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/lambert/lambert.inl"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"

namespace scene::material::substitute {

bxdf::Result Sample_translucent::evaluate_f(float3 const& wi, bool include_back) const noexcept {
    return evaluate<true>(wi, include_back);
}

bxdf::Result Sample_translucent::evaluate_b(float3 const& wi, bool include_back) const noexcept {
    return evaluate<false>(wi, include_back);
}

void Sample_translucent::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    // No side check needed because the material is two-sided by definition.

    float const p = sampler.generate_sample_1D();

    if (thickness_ > 0.f) {
        if (p < 0.5f) {
            float const n_dot_wi = lambert::Isotropic::reflect(diffuse_color_, layer_, sampler,
                                                               result);

            // This is the least attempt we can do at energy conservation
            float const n_dot_wo = layer_.clamp_n_dot(wo_);

            float const f = base_diffuse_fresnel_hack(n_dot_wi, n_dot_wo);

            result.wi *= -1.f;

            float const approximated_distance = thickness_ / n_dot_wi;

            float3 const attenuation = rendering::attenuation(approximated_distance, attenuation_);

            result.reflection *= (n_dot_wi * (1.f - f)) * attenuation;
        } else {
            if (p < 0.75f) {
                diffuse_sample(wo_, sampler, avoid_caustics_, result);
            } else {
                gloss_sample(wo_, sampler, result);
            }
        }

        result.pdf *= 0.5f;
    } else {
        if (p < 0.5f) {
            diffuse_sample(wo_, sampler, avoid_caustics_, result);
        } else {
            gloss_sample(wo_, sampler, result);
        }
    }
}

bool Sample_translucent::is_translucent() const noexcept {
    return thickness_ > 0.f;
}

void Sample_translucent::set_transluceny(float3 const& diffuse_color, float thickness,
                                         float attenuation_distance) noexcept {
    thickness_ = thickness;

    if (thickness > 0.f) {
        attenuation_ = material::extinction_coefficient(diffuse_color, attenuation_distance);
    }
}

template <bool Forward>
bxdf::Result Sample_translucent::evaluate(float3 const& wi, bool /*include_back*/) const noexcept {
    // No side check needed because the material is two-sided by definition.

    // This is a bit complicated to explain:
    // If the material does not have transmission,
    // we will never get a wi which is in the wrong hemisphere,
    // because that case is handled before coming here,
    // so the check is only neccessary for transmissive materials (codified by thickness > 0).
    // On the other hand, if the there is transmission and wi is actullay coming from "behind",
    // then we don't need to calculate the reflection.
    // In the other case, transmission won't be visible and we only need reflection.
    if (thickness_ > 0.f && !same_hemisphere(wi)) {
        float const n_dot_wi = layer_.clamp_reverse_n_dot(wi);

        float const approximated_distance = thickness_ / n_dot_wi;

        float3 const attenuation = rendering::attenuation(approximated_distance, attenuation_);

        // This is the least attempt we can do at energy conservation
        float const n_dot_wo = layer_.clamp_reverse_n_dot(wo_);

        float const f = base_diffuse_fresnel_hack(n_dot_wi, n_dot_wo);

        float const pdf = n_dot_wi * (0.5f * Pi_inv);

        return {(n_dot_wi * Pi_inv * (1.f - f)) * (attenuation * diffuse_color_), pdf};
    }

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    auto result = base_evaluate<Forward>(wi, wo_, h, wo_dot_h, avoid_caustics_);

    if (thickness_ > 0.f) {
        result.pdf *= 0.5f;
    }

    return result;
}

}  // namespace scene::material::substitute
