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

bxdf::Result Sample_translucent::evaluate_f(float3 const& wi) const {
    return evaluate<true>(wi);
}

bxdf::Result Sample_translucent::evaluate_b(float3 const& wi) const {
    return evaluate<false>(wi);
}

void Sample_translucent::sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    // No side check needed because the material is two-sided by definition.

    float const p = sampler.generate_sample_1D(rng);

    if (thickness_ > 0.f) {
        float const t = transparency_;

        if (p < 0.5f) {
            float const n_dot_wi = lambert::Isotropic::reflect(base_.albedo_, layer_, sampler, rng,
                                                               result);

            // This is the least attempt we can do at energy conservation
            float const n_dot_wo = layer_.clamp_n_dot(wo_);

            float const f = base_.base_diffuse_fresnel_hack(n_dot_wi, n_dot_wo);

            result.wi *= -1.f;

            float const approximated_distance = thickness_ / n_dot_wi;

            float3 const attenuation = rendering::attenuation(approximated_distance, attenuation_);

            result.reflection *= (t * n_dot_wi * (1.f - f)) * attenuation;
        } else {
            float const o = 1.f - t;

            if (p < 0.75f) {
                base_.diffuse_sample(wo_, *this, o, sampler, rng, result);
            } else {
                base_.gloss_sample(wo_, *this, o, sampler, rng, result);
            }
        }

        result.pdf *= 0.5f;
    } else {
        if (p < 0.5f) {
            base_.diffuse_sample(wo_, *this, sampler, rng, result);
        } else {
            base_.gloss_sample(wo_, *this, sampler, rng, result);
        }
    }

    result.wavelength = 0.f;
}

void Sample_translucent::set_transluceny(float3 const& color, float thickness,
                                         float attenuation_distance, float transparency) {
    properties_.set(Property::Translucent, thickness > 0.f);

    attenuation_ = material::attenuation_coefficient(color, attenuation_distance);

    thickness_    = thickness;
    transparency_ = transparency;
}

template <bool Forward>
bxdf::Result Sample_translucent::evaluate(float3 const& wi) const {
    // No side check needed because the material is two-sided by definition.

    float const t = transparency_;

    // This is a bit complicated to explain:
    // If the material does not have transmission,
    // we will never get a wi which is in the wrong hemisphere,
    // because that case is handled before coming here,
    // so the check is only neccessary for transmissive materials (codified by thickness > 0).
    // On the other hand, if the there is transmission and wi is actually coming from "behind",
    // then we don't need to calculate the reflection.
    // In the other case, transmission won't be visible and we only need reflection.
    if (thickness_ > 0.f && !same_hemisphere(wi)) {
        float const n_dot_wi = layer_.clamp_abs_n_dot(wi);

        float const approximated_distance = thickness_ / n_dot_wi;

        float3 const attenuation = rendering::attenuation(approximated_distance, attenuation_);

        // This is the least attempt we can do at energy conservation
        float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

        float const f = base_.base_diffuse_fresnel_hack(n_dot_wi, n_dot_wo);

        float const pdf = n_dot_wi * (0.5f * Pi_inv);

        return {(t * n_dot_wi * Pi_inv * (1.f - f)) * (attenuation * base_.albedo_), pdf};
    }

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    float const o = 1.f - t;

    auto result = base_.base_evaluate<Forward>(wi, wo_, h, wo_dot_h, *this, o);

    if (thickness_ > 0.f) {
        result.pdf() *= 0.5f;
    }

    return result;
}

}  // namespace scene::material::substitute
