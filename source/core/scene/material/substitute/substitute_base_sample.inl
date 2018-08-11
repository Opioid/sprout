#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_INL
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_INL

#include "base/math/vector3.inl"
#include "scene/material/coating/coating.inl"
#include "scene/material/disney/disney.inl"
#include "scene/material/material_sample_helper.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

template <typename Diffuse>
const material::Sample::Layer& Sample_base<Diffuse>::base_layer() const noexcept {
    return layer_;
}

template <typename Diffuse>
float3 Sample_base<Diffuse>::radiance() const noexcept {
    return layer_.emission_;
}

template <typename Diffuse>
void Sample_base<Diffuse>::Layer::set(float3 const& color, float3 const& radiance, float ior,
                                      float f0, float alpha, float metallic) noexcept {
    diffuse_color_ = (1.f - metallic) * color;
    f0_            = math::lerp(float3(f0), color, metallic);
    emission_      = radiance;
    ior_           = ior;
    alpha_         = alpha;
    metallic_      = metallic;
}

template <typename Diffuse>
bxdf::Result Sample_base<Diffuse>::Layer::base_evaluate(float3 const& wi, float3 const& wo,
                                                        float3 const& h, float wo_dot_h,
                                                        bool avoid_caustics) const noexcept {
    float const n_dot_wi = clamp_n_dot(wi);
    float const n_dot_wo = clamp_abs_n_dot(wo);

    auto const d = Diffuse::reflection(wo_dot_h, n_dot_wi, n_dot_wo, *this);

    if (avoid_caustics && alpha_ <= ggx::Min_alpha) {
        return {n_dot_wi * d.reflection, d.pdf};
    }

    float const n_dot_h = math::saturate(math::dot(n_, h));

    fresnel::Schlick const schlick(f0_);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, *this,
                                                schlick);

    float const pdf = 0.5f * (d.pdf + ggx.pdf);

    // Apparently weight by (1 - fresnel) is not correct!
    // So here we assume Diffuse has the proper fresnel built in - which Disney does (?)

    return {n_dot_wi * (d.reflection + ggx.reflection), pdf};
}

template <typename Diffuse>
bxdf::Result Sample_base<Diffuse>::Layer::pure_gloss_evaluate(float3 const& wi, float3 const& wo,
                                                              float3 const& h, float wo_dot_h,
                                                              bool avoid_caustics) const noexcept {
    if (avoid_caustics && alpha_ <= ggx::Min_alpha) {
        return {float3(0.f), 0.f};
    }

    float const n_dot_wi = clamp_n_dot(wi);
    float const n_dot_wo = clamp_abs_n_dot(wo);

    float const n_dot_h = math::saturate(math::dot(n_, h));

    fresnel::Schlick const schlick(f0_);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, *this,
                                                schlick);

    // Apparently weight by (1 - fresnel) is not correct!
    // So here we assume Diffuse has the proper fresnel built in - which Disney does (?)

    return {n_dot_wi * ggx.reflection, ggx.pdf};
}

template <typename Diffuse>
void Sample_base<Diffuse>::Layer::diffuse_sample(float3 const& wo, sampler::Sampler& sampler,
                                                 bool avoid_caustics, bxdf::Sample& result) const
    noexcept {
    float const n_dot_wo = clamp_abs_n_dot(wo);
    float const n_dot_wi = Diffuse::reflect(wo, n_dot_wo, *this, sampler, result);

    if (avoid_caustics && alpha_ <= ggx::Min_alpha) {
        result.reflection *= n_dot_wi;
        return;
    }

    float const n_dot_h = math::saturate(math::dot(n_, result.h));

    fresnel::Schlick const schlick(f0_);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, result.h_dot_wi, n_dot_h, *this,
                                                schlick);

    result.reflection = n_dot_wi * (result.reflection + ggx.reflection);
    result.pdf        = 0.5f * (result.pdf + ggx.pdf);
}

template <typename Diffuse>
void Sample_base<Diffuse>::Layer::gloss_sample(float3 const& wo, sampler::Sampler& sampler,
                                               bxdf::Sample& result) const noexcept {
    float const n_dot_wo = clamp_abs_n_dot(wo);

    fresnel::Schlick const schlick(f0_);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, schlick, sampler, result);

    auto const d = Diffuse::reflection(result.h_dot_wi, n_dot_wi, n_dot_wo, *this);

    result.reflection = n_dot_wi * (result.reflection + d.reflection);
    result.pdf        = 0.5f * (result.pdf + d.pdf);
}

template <typename Diffuse>
void Sample_base<Diffuse>::Layer::pure_gloss_sample(float3 const& wo, sampler::Sampler& sampler,
                                                    bxdf::Sample& result) const noexcept {
    float const n_dot_wo = clamp_abs_n_dot(wo);

    fresnel::Schlick const schlick(f0_);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, schlick, sampler, result);
    result.reflection *= n_dot_wi;
}

template <typename Diffuse>
float Sample_base<Diffuse>::Layer::base_diffuse_fresnel_hack(float n_dot_wi, float n_dot_wo) const
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

}  // namespace scene::material::substitute

#endif
