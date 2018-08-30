#ifndef SU_CORE_SCENE_MATERIAL_COATING_COATING_INL
#define SU_CORE_SCENE_MATERIAL_COATING_COATING_INL

#include "coating.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"

namespace scene::material::coating {

inline void Clearcoat::set(float3 const& absorption_coefficient, float thickness, float ior, float f0,
                           float alpha, float weight) noexcept {
    absorption_coefficient_ = absorption_coefficient;

    thickness_ = thickness;

    ior_ = ior;

    f0_ = f0;

    alpha_ = alpha;

    weight_ = weight;
}

template <typename Layer>
Result Clearcoat::evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                           Layer const& layer, bool avoid_caustics) const noexcept {
    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    float const f = weight_ * fresnel::schlick(std::min(n_dot_wi, n_dot_wo), f0_);

    float const d = thickness_ * (1.f / n_dot_wi + 1.f / n_dot_wo);

    float3 const absorption = rendering::attenuation(d, absorption_coefficient_);

    float3 const attenuation = (1.f - f) * absorption;

    if (avoid_caustics && alpha_ <= ggx::Min_alpha) {
        return {float3(0.f), attenuation, 0.f};
    }

    float const n_dot_h = math::saturate(math::dot(layer.n_, h));

    fresnel::Schlick const schlick(f0_);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, layer,
                                                schlick);

    return {weight_ * n_dot_wi * ggx.reflection, attenuation, ggx.pdf};
}

template <typename Layer>
void Clearcoat::sample(float3 const& wo, Layer const& layer, sampler::Sampler& sampler,
                       float3& attenuation, bxdf::Sample& result) const noexcept {
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    fresnel::Schlick const schlick(f0_);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, layer, schlick, sampler, result);

    float const f = weight_ * fresnel::schlick(std::min(n_dot_wi, n_dot_wo), f0_);

    float const d = thickness_ * (1.f / n_dot_wi + 1.f / n_dot_wo);

    float3 const absorption = rendering::attenuation(d, absorption_coefficient_);

    attenuation = (1.f - f) * absorption;

    result.reflection *= weight_ * n_dot_wi;
}

inline void Thinfilm::set(float ior, float ior_internal, float alpha, float thickness) noexcept {
    ior_          = ior;
    ior_internal_ = ior_internal;
    alpha_        = alpha;
    thickness_    = thickness;
}

template <typename Layer>
Result Thinfilm::evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                          Layer const& layer, bool /*avoid_caustics*/) const noexcept {
    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    float const n_dot_h = math::saturate(math::dot(layer.n_, h));

    const fresnel::Thinfilm thinfilm(1.f, ior_, ior_internal_, thickness_);

    float3     fresnel;
    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, layer,
                                                thinfilm, fresnel);

    float3 const attenuation = (1.f - fresnel);

    return {n_dot_wi * ggx.reflection, attenuation, ggx.pdf};
}

template <typename Layer>
void Thinfilm::sample(float3 const& wo, Layer const& layer, sampler::Sampler& sampler,
                      float3& attenuation, bxdf::Sample& result) const noexcept {
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    fresnel::Thinfilm const thinfilm(1.f, ior_, ior_internal_, thickness_);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, layer, thinfilm, sampler,
                                                   attenuation, result);

    attenuation = (1.f - attenuation);

    result.reflection *= n_dot_wi;
}

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
