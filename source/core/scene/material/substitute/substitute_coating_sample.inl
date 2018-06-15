#pragma once

#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"
#include "substitute_coating_sample.hpp"

namespace scene::material::substitute {

template <typename Coating_layer>
bxdf::Result Sample_coating<Coating_layer>::evaluate(f_float3 wi, bool avoid_caustics) const {
    if (!same_hemisphere(wo_)) {
        return {float3::identity(), 0.f};
    }

    return base_and_coating_evaluate(wi, coating_, avoid_caustics);
}

template <typename Coating_layer>
void Sample_coating<Coating_layer>::sample(sampler::Sampler& sampler, bool avoid_caustics,
                                           bxdf::Sample& result) const {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    base_and_coating_sample(coating_, sampler, avoid_caustics, result);

    result.wavelength = 0.f;
}

}  // namespace scene::material::substitute
