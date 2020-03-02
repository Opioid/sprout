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

void Sample_translucent::sample(Sampler& sampler, bxdf::Sample& result) const {
    // No side check needed because the material is two-sided by definition.

    float const p = sampler.generate_sample_1D();

    if (thickness_ > 0.f) {
        float const t = transparency_;

        if (p < t) {
            float const n_dot_wi = lambert::Isotropic::reflect(base_.diffuse_color_, layer_,
                                                               sampler, result);

            // This is the least attempt we can do at energy conservation
            float const n_dot_wo = layer_.clamp_n_dot(wo_);

            float const f = base_.base_diffuse_fresnel_hack(n_dot_wi, n_dot_wo);

            result.wi *= -1.f;

            float const approximated_distance = thickness_ / n_dot_wi;

            float3 const attenuation = rendering::attenuation(approximated_distance, attenuation_);

            result.reflection *= (n_dot_wi * (1.f - f)) * attenuation;

        //  result.pdf *= t;
        } else {
            float const u = 1.f - t;
            // TODO: adjust p for flexible t
            if (p < 0.75f) {
                base_.diffuse_sample(wo_, layer_, sampler, base_.avoid_caustics_, result);
            } else {
                base_.gloss_sample(wo_, layer_, sampler, result);
            }


        }

     //   result.pdf *= 0.5f;
    } else {
        if (p < 0.5f) {
            base_.diffuse_sample(wo_, layer_, sampler, base_.avoid_caustics_, result);
        } else {
            base_.gloss_sample(wo_, layer_, sampler, result);
        }
    }

    result.wavelength = 0.f;
}

bool Sample_translucent::is_translucent() const {
    return thickness_ > 0.f;
}

void Sample_translucent::set_transluceny(float3 const& color, float transparency, float thickness,
                                         float attenuation_distance) {
    attenuation_ = material::extinction_coefficient(color, attenuation_distance);

    transparency_ = transparency;
    thickness_ = thickness;
}

template <bool Forward>
bxdf::Result Sample_translucent::evaluate(float3 const& wi) const {
    // No side check needed because the material is two-sided by definition.

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

        return {(n_dot_wi * Pi_inv * (1.f - f)) * (attenuation * base_.diffuse_color_), pdf};
    }

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    auto result = base_.base_evaluate<Forward>(wi, wo_, h, wo_dot_h, layer_);

    if (thickness_ > 0.f) {
        result.pdf() *= 0.5f;
    }

    return result;
}

}  // namespace scene::material::substitute
