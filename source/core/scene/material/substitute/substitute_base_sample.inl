#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_INL
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_INL

#include "base/math/vector3.inl"
#include "scene/material/coating/coating.inl"
#include "scene/material/disney/disney.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample_helper.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

template <typename Diffuse>
void Base_closure<Diffuse>::set(float3 const& color, float3 const& radiance, float f0, float alpha,
                                float metallic) noexcept {
    diffuse_color_ = (1.f - metallic) * color;

    f0_ = lerp(float3(f0), color, metallic);

    emission_ = radiance;

    metallic_ = metallic;

    alpha_ = alpha;
}

template <typename Diffuse>
template <bool Forward>
bxdf::Result Base_closure<Diffuse>::base_evaluate(float3 const& wi, float3 const& wo,
                                                  float3 const& h, float wo_dot_h,
                                                  Layer const& layer, bool avoid_caustics) const
    noexcept {
    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    auto const d = Diffuse::reflection(wo_dot_h, n_dot_wi, n_dot_wo, alpha_, diffuse_color_);

    if (avoid_caustics && alpha_ <= ggx::Min_alpha) {
        if constexpr (Forward) {
            return {n_dot_wi * d.reflection, d.pdf};
        } else {
            return d;
        }
    }

    float const n_dot_h = saturate(layer.n_dot(h));

    fresnel::Schlick const schlick(f0_);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_,
                                                schlick);

    float const pdf = 0.5f * (d.pdf + ggx.pdf);

    // Apparently weight by (1 - fresnel) is not correct!
    // So here we assume Diffuse has the proper fresnel built in - which Disney does (?)

    if constexpr (Forward) {
        return {n_dot_wi * (d.reflection + ggx.reflection), pdf};
    } else {
        return {d.reflection + ggx.reflection, pdf};
    }
}

template <typename Diffuse>
template <bool Forward>
bxdf::Result Base_closure<Diffuse>::pure_gloss_evaluate(float3 const& wi, float3 const& wo,
                                                        float3 const& h, float wo_dot_h,
                                                        Layer const& layer,
                                                        bool avoid_caustics) const noexcept {
    if (avoid_caustics && alpha_ <= ggx::Min_alpha) {
        return {float3(0.f), 0.f};
    }

    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    float const n_dot_h = saturate(layer.n_dot(h));

    fresnel::Schlick const schlick(f0_);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_,
                                                schlick);

    // Apparently weight by (1 - fresnel) is not correct!
    // So here we assume Diffuse has the proper fresnel built in - which Disney does (?)

    if constexpr (Forward) {
        return {n_dot_wi * ggx.reflection, ggx.pdf};
    } else {
        return ggx;
    }
}

template <typename Diffuse>
void Base_closure<Diffuse>::diffuse_sample(float3 const& wo, Layer const& layer, Sampler& sampler,
                                           bool avoid_caustics, bxdf::Sample& result) const
    noexcept {
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);
    float const n_dot_wi = Diffuse::reflect(wo, n_dot_wo, layer, alpha_, diffuse_color_, sampler,
                                            result);

    if (avoid_caustics && alpha_ <= ggx::Min_alpha) {
        result.reflection *= n_dot_wi;
        return;
    }

    float const n_dot_h = saturate(layer.n_dot(result.h));

    fresnel::Schlick const schlick(f0_);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, result.h_dot_wi, n_dot_h,
                                                alpha_, schlick);

    result.reflection = n_dot_wi * (result.reflection + ggx.reflection);
    result.pdf        = 0.5f * (result.pdf + ggx.pdf);
}

template <typename Diffuse>
void Base_closure<Diffuse>::gloss_sample(float3 const& wo, Layer const& layer, Sampler& sampler,
                                         bxdf::Sample& result) const noexcept {
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    fresnel::Schlick const schlick(f0_);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, layer, alpha_, schlick, sampler,
                                                   result);

    auto const d = Diffuse::reflection(result.h_dot_wi, n_dot_wi, n_dot_wo, alpha_, diffuse_color_);

    result.reflection = n_dot_wi * (result.reflection + d.reflection);
    result.pdf        = 0.5f * (result.pdf + d.pdf);
}

template <typename Diffuse>
void Base_closure<Diffuse>::pure_gloss_sample(float3 const& wo, Layer const& layer,
                                              Sampler& sampler, bxdf::Sample& result) const
    noexcept {
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    fresnel::Schlick const schlick(f0_);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, layer, alpha_, schlick, sampler,
                                                   result);
    result.reflection *= n_dot_wi;
}

template <typename Diffuse>
float Base_closure<Diffuse>::base_diffuse_fresnel_hack(float n_dot_wi, float n_dot_wo) const
    noexcept {
    // I think this is what we have to weigh lambert with if it is added to a microfacet BRDF.
    // At the moment this is only used with the "translucent" material,
    // which is kind of hacky anyway.

    //	float const a = fresnel::schlick(n_dot_wi, f0_[0]);
    //	float const b = fresnel::schlick(n_dot_wo, f0_[0]);
    //	return std::max(a, b);

    // Same as above, but shorter
    return fresnel::schlick(std::min(n_dot_wi, n_dot_wo), f0_[0]);
}

inline const material::Layer& Sample_base::base_layer() const noexcept {
    return layer_;
}

}  // namespace scene::material::substitute

#endif
