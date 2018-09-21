#ifndef SU_CORE_SCENE_MATERIAL_COATING_COATING_INL
#define SU_CORE_SCENE_MATERIAL_COATING_COATING_INL

#include "coating.hpp"

namespace scene::material::coating {

template <typename Coating>
Result Coating_layer<Coating>::evaluate(float3 const& wi, float3 const& wo, float3 const& h,
                                        float wo_dot_h, bool avoid_caustics) const noexcept {
    return Coating::evaluate(wi, wo, h, wo_dot_h, *this, avoid_caustics);
}

template <typename Coating>
void Coating_layer<Coating>::sample(float3 const& wo, sampler::Sampler& sampler,
                                    float3& attenuation, bxdf::Sample& result) const noexcept {
    Coating::sample(wo, *this, sampler, attenuation, result);
}

}  // namespace scene::material::coating

#endif
