#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_INL
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_INL

#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"
#include "substitute_coating_sample.hpp"

namespace scene::material::substitute {

template <typename Coating_layer>
bxdf::Result Sample_coating<Coating_layer>::evaluate(float3 const& wi) const {
    if (!same_hemisphere(wo_)) {
        return {float3::identity(), 0.f};
    }

    return base_and_coating_evaluate(wi, coating_);
}

template <typename Coating_layer>
void Sample_coating<Coating_layer>::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    base_and_coating_sample(coating_, sampler, result);

    result.wavelength = 0.f;
}

}  // namespace scene::material::substitute

#endif
