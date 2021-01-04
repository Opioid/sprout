#include "substitute_sample.hpp"
#include "base/math/math.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"

namespace scene::material::substitute {

bxdf::Result Sample::evaluate(float3_p wi) const {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    if (1.f == base_.metallic_) {
        return base_.pure_gloss_evaluate(wi, wo_, h, wo_dot_h, *this);
    }

    return base_.base_evaluate(wi, wo_, h, wo_dot_h, *this);
}

void Sample::sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    if (1.f == base_.metallic_) {
        base_.pure_gloss_sample(wo_, *this, sampler, rng, result);
    } else {
        float const p = sampler.sample_1D(rng);

        if (p < 0.5f) {
            base_.diffuse_sample(wo_, *this, sampler, rng, result);
        } else {
            base_.gloss_sample(wo_, *this, sampler, rng, result);
        }
    }

    result.wavelength = 0.f;
}

}  // namespace scene::material::substitute
