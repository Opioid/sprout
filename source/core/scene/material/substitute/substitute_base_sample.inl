#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_INL
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_INL

#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/coating/coating.inl"
#include "scene/material/disney/disney.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample_helper.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

using Mat_sample = material::Sample;

template <typename Diff>
void Base_closure<Diff>::set(float3_p color, float f0, float metallic) {
    albedo_ = (1.f - metallic) * color;

    f0_ = lerp(float3(f0), color, metallic);

    metallic_ = metallic;
}

template <typename Diff>
template <bool Forward>
bxdf::Result Base_closure<Diff>::base_evaluate(float3_p wi, float3_p wo, float3_p h, float wo_dot_h,
                                               Mat_sample const& sample) const {
    Layer const& layer = sample.layer_;

    float const alpha = sample.alpha_;

    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    auto const d = Diff::reflection(wo_dot_h, n_dot_wi, n_dot_wo, alpha, albedo_);

    if (sample.avoid_caustics() && alpha <= ggx::Min_alpha) {
        if constexpr (Forward) {
            return {n_dot_wi * d.reflection, d.pdf()};
        } else {
            return d;
        }
    }

    float const n_dot_h = saturate(layer.n_dot(h));

    fresnel::Schlick const schlick(f0_);

    auto ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha, schlick);

    ggx.reflection *= ggx::ilm_ep_conductor(f0_, n_dot_wo, alpha);

    float const pdf = 0.5f * (d.pdf() + ggx.pdf());

    // Apparently weight by (1 - fresnel) is not correct!
    // So here we assume diffuse has the proper fresnel built in - which Disney does (?)

    if constexpr (Forward) {
        return {n_dot_wi * (d.reflection + ggx.reflection), pdf};
    } else {
        return {d.reflection + ggx.reflection, pdf};
    }
}

template <typename Diff>
template <bool Forward>
bxdf::Result Base_closure<Diff>::base_evaluate(float3_p wi, float3_p wo, float3_p h, float wo_dot_h,
                                               Mat_sample const& sample, float diff_factor) const {
    Layer const& layer = sample.layer_;

    float const alpha = sample.alpha_;

    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    auto const d = Diff::reflection(wo_dot_h, n_dot_wi, n_dot_wo, alpha, diff_factor * albedo_);

    if (sample.avoid_caustics() && alpha <= ggx::Min_alpha) {
        if constexpr (Forward) {
            return {n_dot_wi * d.reflection, d.pdf()};
        } else {
            return d;
        }
    }

    float const n_dot_h = saturate(layer.n_dot(h));

    fresnel::Schlick const schlick(f0_);

    auto ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha, schlick);

    ggx.reflection *= ggx::ilm_ep_conductor(f0_, n_dot_wo, alpha);

    float const pdf = 0.5f * (d.pdf() + ggx.pdf());

    // Apparently weight by (1 - fresnel) is not correct!
    // So here we assume diffuse has the proper fresnel built in - which Disney does (?)

    if constexpr (Forward) {
        return {n_dot_wi * (d.reflection + ggx.reflection), pdf};
    } else {
        return {d.reflection + ggx.reflection, pdf};
    }
}

template <typename Diff>
template <bool Forward>
bxdf::Result Base_closure<Diff>::pure_gloss_evaluate(float3_p wi, float3_p wo, float3_p h,
                                                     float             wo_dot_h,
                                                     Mat_sample const& sample) const {
    Layer const& layer = sample.layer_;

    float const alpha = sample.alpha_;

    if (sample.avoid_caustics() & (alpha <= ggx::Min_alpha)) {
        return {float3(0.f), 0.f};
    }

    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    float const n_dot_h = saturate(layer.n_dot(h));

    fresnel::Schlick const schlick(f0_);

    auto ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha, schlick);

    ggx.reflection *= ggx::ilm_ep_conductor(f0_, n_dot_wo, alpha);

    // Apparently weight by (1 - fresnel) is not correct!
    // So here we assume diffuse has the proper fresnel built in - which Disney does (?)

    if constexpr (Forward) {
        return {n_dot_wi * ggx.reflection, ggx.pdf()};
    } else {
        return ggx;
    }
}

template <typename Diff>
void Base_closure<Diff>::diffuse_sample(float3_p wo, Mat_sample const& sample, Sampler& sampler,
                                        RNG& rng, bxdf::Sample& result) const {
    Layer const& layer = sample.layer_;

    float const alpha = sample.alpha_;

    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    float2 const xi = sampler.sample_2D(rng);

    float const n_dot_wi = Diff::reflect(wo, n_dot_wo, layer, alpha, albedo_, xi, result);

    if (sample.avoid_caustics() & (alpha <= ggx::Min_alpha)) {
        result.reflection *= n_dot_wi;
        return;
    }

    float const n_dot_h = saturate(layer.n_dot(result.h));

    fresnel::Schlick const schlick(f0_);

    auto ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, result.h_dot_wi, n_dot_h, alpha,
                                          schlick);

    ggx.reflection *= ggx::ilm_ep_conductor(f0_, n_dot_wo, alpha);

    result.reflection = n_dot_wi * (result.reflection + ggx.reflection);
    result.pdf        = 0.5f * (result.pdf + ggx.pdf());
}

template <typename Diff>
void Base_closure<Diff>::diffuse_sample(float3_p wo, Mat_sample const& sample, float diff_factor,
                                        Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    Layer const& layer = sample.layer_;

    float const alpha = sample.alpha_;

    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    float2 const xi = sampler.sample_2D(rng);

    float const n_dot_wi = Diff::reflect(wo, n_dot_wo, layer, alpha, diff_factor * albedo_, xi,
                                         result);

    if (sample.avoid_caustics() & (alpha <= ggx::Min_alpha)) {
        result.reflection *= n_dot_wi;
        return;
    }

    float const n_dot_h = saturate(layer.n_dot(result.h));

    fresnel::Schlick const schlick(f0_);

    auto ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, result.h_dot_wi, n_dot_h, alpha,
                                          schlick);

    ggx.reflection *= ggx::ilm_ep_conductor(f0_, n_dot_wo, alpha);

    result.reflection = n_dot_wi * (result.reflection + ggx.reflection);
    result.pdf        = 0.5f * (result.pdf + ggx.pdf());
}

template <typename Diff>
void Base_closure<Diff>::gloss_sample(float3_p wo, Mat_sample const& sample, Sampler& sampler,
                                      RNG& rng, bxdf::Sample& result) const {
    Layer const& layer = sample.layer_;

    float const alpha = sample.alpha_;

    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    fresnel::Schlick const schlick(f0_);

    float2 const xi = sampler.sample_2D(rng);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, alpha, schlick, xi, layer, result);

    result.reflection *= ggx::ilm_ep_conductor(f0_, n_dot_wo, alpha);

    auto const d = Diff::reflection(result.h_dot_wi, n_dot_wi, n_dot_wo, alpha, albedo_);

    result.reflection = n_dot_wi * (result.reflection + d.reflection);
    result.pdf        = 0.5f * (result.pdf + d.pdf());
}

template <typename Diff>
void Base_closure<Diff>::gloss_sample(float3_p wo, Mat_sample const& sample, float diff_factor,
                                      Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    Layer const& layer = sample.layer_;

    float const alpha = sample.alpha_;

    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    fresnel::Schlick const schlick(f0_);

    float2 const xi = sampler.sample_2D(rng);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, alpha, schlick, xi, layer, result);

    result.reflection *= ggx::ilm_ep_conductor(f0_, n_dot_wo, alpha);

    auto const d = Diff::reflection(result.h_dot_wi, n_dot_wi, n_dot_wo, alpha,
                                    diff_factor * albedo_);

    result.reflection = n_dot_wi * (result.reflection + d.reflection);
    result.pdf        = 0.5f * (result.pdf + d.pdf());
}

template <typename Diff>
void Base_closure<Diff>::pure_gloss_sample(float3_p wo, Mat_sample const& sample, Sampler& sampler,
                                           RNG& rng, bxdf::Sample& result) const {
    Layer const& layer = sample.layer_;

    float const alpha = sample.alpha_;

    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    fresnel::Schlick const schlick(f0_);

    float2 const xi = sampler.sample_2D(rng);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, alpha, schlick, xi, layer, result);

    result.reflection *= n_dot_wi * ggx::ilm_ep_conductor(f0_, n_dot_wo, alpha);
}

template <typename Diff>
float Base_closure<Diff>::base_diffuse_fresnel_hack(float n_dot_wi, float n_dot_wo) const {
    // I think this is what we have to weigh lambert with, if it is added to a microfacet BRDF.
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
