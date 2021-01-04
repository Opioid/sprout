#include "coating.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"

namespace scene::material::coating {

void Clearcoat::set(float3_p absorption_coef, float thickness, float ior, float f0, float alpha,
                    float weight) {
    absorption_coef_ = absorption_coef;

    thickness_ = thickness;

    ior_ = ior;

    f0_ = f0;

    alpha_ = alpha;

    weight_ = weight;
}

float3 Clearcoat::attenuation(float n_dot_wo) const {
    float const d = thickness_ * (1.f / n_dot_wo);

    return rendering::attenuation(d, absorption_coef_);
}

float3 Clearcoat::attenuation(float n_dot_wi, float n_dot_wo) const {
    float const f = weight_ * fresnel::schlick(std::min(n_dot_wi, n_dot_wo), f0_);

    float const d = thickness_ * (1.f / n_dot_wi + 1.f / n_dot_wo);

    float3 const absorption = rendering::attenuation(d, absorption_coef_);

    float3 const attenuation = (1.f - f) * absorption;

    return attenuation;
}

Result Clearcoat::evaluate(float3_p wi, float3_p wo, float3_p h, float wo_dot_h, Layer const& layer,
                           bool avoid_caustics) const {
    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    float3 const attenuation = Clearcoat::attenuation(n_dot_wi, n_dot_wo);

    if (avoid_caustics && alpha_ <= ggx::Min_alpha) {
        return {float3(0.f), attenuation, 0.f, 0.f};
    }

    float const n_dot_h = saturate(dot(layer.n_, h));

    fresnel::Schlick const schlick(f0_);

    float3     fresnel_result;
    auto const ggx = ggx::Iso::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_, schlick,
                                          fresnel_result);

    float const ep = ggx::ilm_ep_dielectric(n_dot_wo, alpha_, ior_);

    return {ep * weight_ * n_dot_wi * ggx.reflection, attenuation, fresnel_result[0], ggx.pdf()};
}

Result Clearcoat::evaluate_b(float3_p wi, float3_p wo, float3_p h, float wo_dot_h,
                             Layer const& layer, bool avoid_caustics) const {
    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    float3 const attenuation = Clearcoat::attenuation(n_dot_wi, n_dot_wo);

    if (avoid_caustics && alpha_ <= ggx::Min_alpha) {
        return {float3(0.f), attenuation, 0.f, 0.f};
    }

    float const n_dot_h = saturate(dot(layer.n_, h));

    fresnel::Schlick const schlick(f0_);

    float3     fresnel_result;
    auto const ggx = ggx::Iso::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_, schlick,
                                          fresnel_result);

    float const ep = ggx::ilm_ep_dielectric(n_dot_wo, alpha_, ior_);

    return {ep * weight_ * ggx.reflection, attenuation, fresnel_result[0], ggx.pdf()};
}

void Clearcoat::reflect(float3_p wo, float3_p h, float n_dot_wo, float n_dot_h, float wi_dot_h,
                        float wo_dot_h, Layer const& layer, float3& attenuation,
                        bxdf::Sample& result) const {
    float3 const fresnel = result.reflection;

    float const n_dot_wi = ggx::Iso::reflect(wo, h, n_dot_wo, n_dot_h, wi_dot_h, wo_dot_h, alpha_,
                                             layer, result);

    attenuation = Clearcoat::attenuation(n_dot_wi, n_dot_wo);

    float const ep = ggx::ilm_ep_dielectric(n_dot_wo, alpha_, ior_);

    result.reflection *= ep * weight_ * n_dot_wi * fresnel;
}

float Clearcoat::sample(float3_p wo, Layer const& layer, Sampler& sampler, RNG& rng, float& n_dot_h,
                        bxdf::Sample& result) const {
    float2 const xi = sampler.sample_2D(rng, 1);

    float3 const h = ggx::Iso::sample(wo, alpha_, xi, layer, n_dot_h);

    float const wo_dot_h = clamp_dot(wo, h);

    float const f = fresnel::schlick(wo_dot_h, f0_);

    result.reflection = float3(f);
    result.h          = h;
    result.h_dot_wi   = wo_dot_h;

    return f;
}

void Thinfilm::set(float ior, float ior_internal, float alpha, float thickness) {
    ior_          = ior;
    ior_internal_ = ior_internal;
    alpha_        = alpha;
    thickness_    = thickness;
}

float3 Thinfilm::attenuation(float /*n_dot*/) const {
    return float3(1.f);
}

Result Thinfilm::evaluate(float3_p wi, float3_p wo, float3_p h, float wo_dot_h, Layer const& layer,
                          bool /*avoid_caustics*/) const {
    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    float const n_dot_h = saturate(dot(layer.n_, h));

    const fresnel::Thinfilm thinfilm(1.f, ior_, ior_internal_, thickness_);

    float3     fresnel;
    auto const ggx = ggx::Iso::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_, thinfilm,
                                          fresnel);

    float3 const attenuation = (1.f - fresnel);

    return {n_dot_wi * ggx.reflection, attenuation, max_component(fresnel), ggx.pdf()};
}

Result Thinfilm::evaluate_b(float3_p wi, float3_p wo, float3_p h, float wo_dot_h,
                            Layer const& layer, bool /*avoid_caustics*/) const {
    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo);

    float const n_dot_h = saturate(dot(layer.n_, h));

    const fresnel::Thinfilm thinfilm(1.f, ior_, ior_internal_, thickness_);

    float3     fresnel;
    auto const ggx = ggx::Iso::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_, thinfilm,
                                          fresnel);

    float3 const attenuation = (1.f - fresnel);

    return {ggx.reflection, attenuation, max_component(fresnel), ggx.pdf()};
}

void Thinfilm::reflect(float3_p wo, float3_p h, float n_dot_wo, float n_dot_h, float wi_dot_h,
                       float wo_dot_h, Layer const& layer, float3& attenuation,
                       bxdf::Sample& result) const {
    float3 const fresnel = result.reflection;

    float const n_dot_wi = ggx::Iso::reflect(wo, h, n_dot_wo, n_dot_h, wi_dot_h, wo_dot_h, alpha_,
                                             layer, result);

    attenuation = 1 - fresnel;

    result.reflection *= n_dot_wi * fresnel;
}

float Thinfilm::sample(float3_p wo, Layer const& layer, Sampler& sampler, RNG& rng, float& n_dot_h,
                       bxdf::Sample& result) const {
    float2 const xi = sampler.sample_2D(rng, 1);

    float3 const h = ggx::Iso::sample(wo, alpha_, xi, layer, n_dot_h);

    float const wo_dot_h = clamp_dot(wo, h);

    float3 const f = fresnel::thinfilm(wo_dot_h, 1.f, ior_, ior_internal_, thickness_);

    result.reflection = float3(f);
    result.h          = h;
    result.h_dot_wi   = wo_dot_h;

    return max_component(f);
}

}  // namespace scene::material::coating
