#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_INL
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_INL

#include "base/math/vector3.inl"
#include "scene/material/coating/coating.inl"
#include "scene/material/disney/disney.inl"
#include "scene/material/material_sample_helper.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

template <typename Diffuse>
const material::Sample::Layer& Sample_base<Diffuse>::base_layer() const {
    return layer_;
}

template <typename Diffuse>
float3 Sample_base<Diffuse>::radiance() const {
    return layer_.emission_;
}

template <typename Diffuse>
template <typename Coating>
bxdf::Result Sample_base<Diffuse>::base_and_coating_evaluate(float3 const&  wi,
                                                             Coating const& coating_layer) const {
    float3 const h = math::normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    auto const coating = coating_layer.evaluate(wi, wo_, h, wo_dot_h, layer_.ior_);

    auto const base = layer_.base_evaluate(wi, wo_, h, wo_dot_h, avoid_caustics_);

    float const pdf = (coating.pdf + 2.f * base.pdf) / 3.f;
    return {coating.reflection + coating.attenuation * base.reflection, pdf};
}

template <typename Diffuse>
template <typename Coating>
void Sample_base<Diffuse>::base_and_coating_sample(Coating const&    coating_layer,
                                                   sampler::Sampler& sampler,
                                                   bxdf::Sample&     result) const {
    float const p = sampler.generate_sample_1D();

    if (p < 0.5f) {
        float3 coating_attenuation;
        coating_layer.sample(wo_, layer_.ior_, sampler, coating_attenuation, result);

        auto const base = layer_.base_evaluate(result.wi, wo_, result.h, result.h_dot_wi,
                                               avoid_caustics_);

        result.reflection = result.reflection + coating_attenuation * base.reflection;
        result.pdf        = (result.pdf + 2.f * base.pdf) / 3.f;
    } else {
        if (1.f == layer_.metallic_) {
            pure_specular_sample_and_coating(coating_layer, sampler, result);
        } else {
            if (p < 0.75f) {
                diffuse_sample_and_coating(coating_layer, sampler, result);
            } else {
                specular_sample_and_coating(coating_layer, sampler, result);
            }
        }
    }
}

template <typename Diffuse>
template <typename Coating>
void Sample_base<Diffuse>::diffuse_sample_and_coating(Coating const&    coating_layer,
                                                      sampler::Sampler& sampler,
                                                      bxdf::Sample&     result) const {
    layer_.diffuse_sample(wo_, sampler, avoid_caustics_, result);

    auto const coating = coating_layer.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
                                                layer_.ior_);

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = (2.f * result.pdf + coating.pdf) / 3.f;
}

template <typename Diffuse>
template <typename Coating>
void Sample_base<Diffuse>::specular_sample_and_coating(Coating const&    coating_layer,
                                                       sampler::Sampler& sampler,
                                                       bxdf::Sample&     result) const {
    layer_.specular_sample(wo_, sampler, result);

    auto const coating = coating_layer.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
                                                layer_.ior_);

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = (2.f * result.pdf + coating.pdf) / 3.f;
}

template <typename Diffuse>
template <typename Coating>
void Sample_base<Diffuse>::pure_specular_sample_and_coating(Coating const&    coating_layer,
                                                            sampler::Sampler& sampler,
                                                            bxdf::Sample&     result) const {
    layer_.pure_specular_sample(wo_, sampler, result);

    auto const coating = coating_layer.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
                                                layer_.ior_);

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = 0.5f * (result.pdf + coating.pdf);
}

template <typename Diffuse>
void Sample_base<Diffuse>::Layer::set(float3 const& color, float3 const& radiance, float ior,
                                      float f0, float roughness, float metallic) {
    diffuse_color_ = (1.f - metallic) * color;
    f0_            = math::lerp(float3(f0), color, metallic);
    emission_      = radiance;
    ior_           = ior;
    roughness_     = roughness;

    float const alpha = roughness * roughness;

    alpha_    = alpha;
    alpha2_   = alpha * alpha;
    metallic_ = metallic;
}

template <typename Diffuse>
bxdf::Result Sample_base<Diffuse>::Layer::base_evaluate(float3 const& wi, float3 const& wo,
                                                        float3 const& h, float wo_dot_h,
                                                        bool avoid_caustics) const {
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
void Sample_base<Diffuse>::Layer::diffuse_sample(float3 const& wo, sampler::Sampler& sampler,
                                                 bool avoid_caustics, bxdf::Sample& result) const {
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
void Sample_base<Diffuse>::Layer::specular_sample(float3 const& wo, sampler::Sampler& sampler,
                                                  bxdf::Sample& result) const {
    float const n_dot_wo = clamp_abs_n_dot(wo);

    fresnel::Schlick const schlick(f0_);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, schlick, sampler, result);

    auto const d = Diffuse::reflection(result.h_dot_wi, n_dot_wi, n_dot_wo, *this);

    result.reflection = n_dot_wi * (result.reflection + d.reflection);
    result.pdf        = 0.5f * (result.pdf + d.pdf);
}

template <typename Diffuse>
void Sample_base<Diffuse>::Layer::pure_specular_sample(float3 const& wo, sampler::Sampler& sampler,
                                                       bxdf::Sample& result) const {
    float const n_dot_wo = clamp_abs_n_dot(wo);

    fresnel::Schlick const schlick(f0_);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, schlick, sampler, result);
    result.reflection *= n_dot_wi;
}

template <typename Diffuse>
float Sample_base<Diffuse>::Layer::base_diffuse_fresnel_hack(float n_dot_wi, float n_dot_wo) const {
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
