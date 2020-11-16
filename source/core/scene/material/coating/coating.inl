#ifndef SU_CORE_SCENE_MATERIAL_COATING_COATING_INL
#define SU_CORE_SCENE_MATERIAL_COATING_COATING_INL

#include "coating.hpp"

namespace scene::material::coating {

template <typename Coating>
Result Coating_layer<Coating>::evaluate_f(float3_p wi, float3_p wo, float3_p h, float wo_dot_h,
                                          bool avoid_caustics) const {
    return Coating::evaluate_f(wi, wo, h, wo_dot_h, *this, avoid_caustics);
}

template <typename Coating>
Result Coating_layer<Coating>::evaluate_b(float3_p wi, float3_p wo, float3_p h, float wo_dot_h,
                                          bool avoid_caustics) const {
    return Coating::evaluate_b(wi, wo, h, wo_dot_h, *this, avoid_caustics);
}

template <typename Coating>
void Coating_layer<Coating>::sample(float3_p wo, Sampler& sampler, RNG& rng, float3& attenuation,
                                    bxdf::Sample& result) const {
    Coating::sample(wo, *this, sampler, rng, attenuation, result);
}

}  // namespace scene::material::coating

#endif
