#include "substitute_sample.hpp"
#include "base/math/math.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"

namespace scene::material::substitute {

bxdf::Result Sample::evaluate_f(float3 const& wi, bool /*include_back*/) const noexcept {
    return evaluate<true>(wi);
}

bxdf::Result Sample::evaluate_b(float3 const& wi, bool /*include_back*/) const noexcept {
    return evaluate<false>(wi);
}

void Sample::sample(Sampler& sampler, bxdf::Sample& result) const noexcept {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    if (1.f == base_.metallic_) {
        base_.pure_gloss_sample(wo_, layer_, sampler, result);
    } else {
        float const p = sampler.generate_sample_1D();

        if (p < 0.5f) {
            base_.diffuse_sample(wo_, layer_, sampler, base_.avoid_caustics_, result);
        } else {
            base_.gloss_sample(wo_, layer_, sampler, result);
        }
    }

    result.wavelength = 0.f;
}

template <bool Forward>
bxdf::Result Sample::evaluate(float3 const& wi) const noexcept {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    if (1.f == base_.metallic_) {
        return base_.pure_gloss_evaluate<Forward>(wi, wo_, h, wo_dot_h, layer_);
    }

    return base_.base_evaluate<Forward>(wi, wo_, h, wo_dot_h, layer_);
}

}  // namespace scene::material::substitute
