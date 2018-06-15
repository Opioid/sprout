#include "substitute_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/sampling/sampling.hpp"
#include "base/math/vector4.inl"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"

namespace scene::material::substitute {

bxdf::Result Sample::evaluate(f_float3 wi, bool avoid_caustics) const {
    if (!same_hemisphere(wo_)) {
        return {float3::identity(), 0.f};
    }

    float3 const h        = math::normalize(wo_ + wi);
    float const  wo_dot_h = clamp_dot(wo_, h);

    return layer_.base_evaluate(wi, wo_, h, wo_dot_h, avoid_caustics);
}

void Sample::sample(sampler::Sampler& sampler, bool avoid_caustics, bxdf::Sample& result) const {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    if (1.f == layer_.metallic_) {
        layer_.pure_specular_sample(wo_, sampler, result);
    } else {
        float const p = sampler.generate_sample_1D();

        if (p < 0.5f) {
            layer_.diffuse_sample(wo_, sampler, avoid_caustics, result);
        } else {
            layer_.specular_sample(wo_, sampler, result);
        }
    }

    result.wavelength = 0.f;
}

}  // namespace scene::material::substitute
