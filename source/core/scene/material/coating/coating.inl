#pragma once

#include "coating.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"

namespace scene::material::coating {

inline void Coating_base::set_color_and_weight(float3 const& color, float weight) noexcept {
    color_  = color;
    weight_ = weight;
}

inline void Clearcoat::set(float f0, float alpha, float alpha2) noexcept {
    f0_     = f0;
    alpha_  = alpha;
    alpha2_ = alpha2;
}

template <typename Layer>
Result Clearcoat::evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                           Layer const& layer, bool avoid_caustics) const noexcept {
    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    float const a = weight_ * fresnel::schlick(std::min(n_dot_wi, n_dot_wo), f0_);

    float3 const attenuation = (1.f - a) * math::lerp(float3(1.f), color_, weight_);

    if (avoid_caustics && alpha_ <= ggx::Min_alpha) {
        return {float3(0.f), attenuation, 0.f};
    }

    float const n_dot_h = math::saturate(math::dot(layer.n_, h));

    fresnel::Schlick const schlick(f0_);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, layer,
                                                schlick);

    return {n_dot_wi * weight_ * ggx.reflection, attenuation, ggx.pdf};
}

template <typename Layer>
void Clearcoat::sample(float3 const& wo, Layer const& layer, sampler::Sampler& sampler,
                       float3& attenuation, bxdf::Sample& result) const noexcept {
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    fresnel::Schlick const schlick(f0_);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, layer, schlick, sampler, result);

    float const a = weight_ * fresnel::schlick(std::min(n_dot_wi, n_dot_wo), f0_);

    attenuation = (1.f - a) * math::lerp(float3(1.f), color_, weight_);

    result.reflection *= n_dot_wi * weight_;
}

inline void Thinfilm::set(float ior, float ior_internal, float alpha, float alpha2,
                          float thickness) noexcept {
    ior_          = ior;
    ior_internal_ = ior_internal;
    alpha_        = alpha;
    alpha2_       = alpha2;
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

    float3 const attenuation = (1.f - fresnel) * math::lerp(float3(1.f), color_, weight_);

    return {n_dot_wi * weight_ * ggx.reflection, attenuation, ggx.pdf};
}

template <typename Layer>
void Thinfilm::sample(float3 const& wo, Layer const& layer, sampler::Sampler& sampler,
                      float3& attenuation, bxdf::Sample& result) const noexcept {
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    fresnel::Thinfilm const thinfilm(1.f, ior_, ior_internal_, thickness_);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, layer, thinfilm, sampler,
                                                   attenuation, result);

    attenuation = (1.f - attenuation) * math::lerp(float3(1.f), color_, weight_);

    result.reflection *= n_dot_wi * weight_;
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
