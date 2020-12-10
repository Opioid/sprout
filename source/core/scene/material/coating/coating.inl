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
void Coating_layer<Coating>::reflect(float3_p wo, float3_p h, float n_dot_wo, float n_dot_h,
                                     float wi_dot_h, float wo_dot_h, float3& attenuation,
                                     bxdf::Sample& result) const {
    Coating::reflect(wo, h, n_dot_wo, n_dot_h, wi_dot_h, wo_dot_h, *this, attenuation, result);
}

template <typename Coating>
float Coating_layer<Coating>::sample(float3_p wo, Sampler& sampler, RNG& rng, float& n_dot_h,
                                     bxdf::Sample& result) const {
    return Coating::sample(wo, *this, sampler, rng, n_dot_h, result);
}

}  // namespace scene::material::coating

#endif
