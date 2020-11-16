#include "metallic_paint_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/coating/coating.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"

namespace scene::material::metallic_paint {

bxdf::Result Sample::evaluate_f(float3_p wi) const {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    auto const coating = coating_.evaluate_f(wi, wo_, h, wo_dot_h, avoid_caustics());

    float3     flakes_fresnel;
    auto const flakes = flakes_.evaluate<true>(wi, wo_, h, wo_dot_h, flakes_fresnel);

    auto const base = base_.evaluate<true>(wi, wo_, h, wo_dot_h);

    float3 const bottom = (1.f - flakes_fresnel) * base.reflection + flakes.reflection;

    float const pdf = (coating.pdf + flakes.pdf() + base.pdf()) / 3.f;

    return {coating.reflection + coating.attenuation * bottom, pdf};
}

bxdf::Result Sample::evaluate_b(float3_p wi) const {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    auto const coating = coating_.evaluate_b(wi, wo_, h, wo_dot_h, avoid_caustics());

    float3     flakes_fresnel;
    auto const flakes = flakes_.evaluate<false>(wi, wo_, h, wo_dot_h, flakes_fresnel);

    auto const base = base_.evaluate<false>(wi, wo_, h, wo_dot_h);

    float3 const bottom = (1.f - flakes_fresnel) * base.reflection + flakes.reflection;

    float const pdf = (coating.pdf + flakes.pdf() + base.pdf()) / 3.f;

    return {coating.reflection + coating.attenuation * bottom, pdf};
}

void Sample::sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const p = sampler.sample_1D(rng);

    if (p < 0.4f) {
        float3 coating_attenuation;
        coating_.sample(wo_, sampler, rng, coating_attenuation, result);

        float3     flakes_fresnel;
        auto const flakes = flakes_.evaluate<true>(result.wi, wo_, result.h, result.h_dot_wi,
                                                   flakes_fresnel);

        auto const base = base_.evaluate<true>(result.wi, wo_, result.h, result.h_dot_wi);

        float3 const bottom = (1.f - flakes_fresnel) * base.reflection + flakes.reflection;

        result.reflection = result.reflection + coating_attenuation * bottom;
        result.pdf        = (result.pdf + base.pdf() + flakes.pdf()) * 0.4f;
    } else if (p < 0.8f) {
        base_.sample(wo_, sampler, rng, result);

        auto const coating = coating_.evaluate_f(result.wi, wo_, result.h, result.h_dot_wi,
                                                 avoid_caustics());

        float3     flakes_fresnel;
        auto const flakes = flakes_.evaluate<true>(result.wi, wo_, result.h, result.h_dot_wi,
                                                   flakes_fresnel);

        float3 const bottom = (1.f - flakes_fresnel) * result.reflection + flakes.reflection;

        result.reflection = coating.reflection + coating.attenuation * bottom;
        result.pdf        = (result.pdf + coating.pdf + flakes.pdf()) * 0.4f;
    } else {
        float3 flakes_fresnel;
        flakes_.sample(wo_, sampler, rng, flakes_fresnel, result);

        auto const coating = coating_.evaluate_f(result.wi, wo_, result.h, result.h_dot_wi,
                                                 avoid_caustics());

        auto const base = base_.evaluate<true>(result.wi, wo_, result.h, result.h_dot_wi);

        float3 const bottom = (1.f - flakes_fresnel) * base.reflection + result.reflection;

        result.reflection = coating.reflection + coating.attenuation * bottom;
        result.pdf        = (result.pdf + base.pdf() + coating.pdf) * 0.2f;
    }
}

void Sample::Base_layer::set(float3_p color_a, float3_p color_b, float alpha) {
    color_a_ = color_a;
    color_b_ = color_b;
    alpha_   = alpha;
}

template <bool Forward>
bxdf::Result Sample::Base_layer::evaluate(float3_p wi, float3_p wo, float3_p h,
                                          float wo_dot_h) const {
    float const n_dot_wi = clamp_n_dot(wi);
    float const n_dot_wo = clamp_abs_n_dot(wo);

    float const f = n_dot_wo;

    float3 const color = lerp(color_b_, color_a_, f);

    float const n_dot_h = saturate(dot(n_, h));

    fresnel::Schlick const fresnel(color);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_,
                                                fresnel);

    if constexpr (Forward) {
        return {n_dot_wi * ggx.reflection, ggx.pdf()};
    } else {
        return ggx;
    }
}

void Sample::Base_layer::sample(float3_p wo, Sampler& sampler, RNG& rng,
                                bxdf::Sample& result) const {
    float const n_dot_wo = clamp_abs_n_dot(wo);

    float const f = n_dot_wo;

    float3 const color = lerp(color_b_, color_a_, f);

    fresnel::Schlick const fresnel(color);

    float2 const xi = sampler.sample_2D(rng);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, alpha_, fresnel, xi,
                                                   result);
    result.reflection *= n_dot_wi;
}

void Sample::Flakes_layer::set(float3_p ior, float3_p absorption, float alpha, float weight) {
    ior_        = ior;
    absorption_ = absorption;
    alpha_      = alpha;
    weight_     = weight;
}

template <bool Forward>
bxdf::Result Sample::Flakes_layer::evaluate(float3_p wi, float3_p wo, float3_p h, float wo_dot_h,
                                            float3& fresnel_result) const {
    float const n_dot_wi = clamp_n_dot(wi);
    float const n_dot_wo = clamp_abs_n_dot(wo);

    float const n_dot_h = saturate(dot(n_, h));

    fresnel::Conductor const conductor(ior_, absorption_);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_,
                                                conductor, fresnel_result);

    fresnel_result *= weight_;

    if constexpr (Forward) {
        return {n_dot_wi * weight_ * ggx.reflection, ggx.pdf()};
    } else {
        return {weight_ * ggx.reflection, ggx.pdf()};
    }
}

void Sample::Flakes_layer::sample(float3_p wo, Sampler& sampler, RNG& rng, float3& fresnel_result,
                                  bxdf::Sample& result) const {
    float const n_dot_wo = clamp_abs_n_dot(wo);

    fresnel::Conductor const conductor(ior_, absorption_);

    float2 const xi = sampler.sample_2D(rng);

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, alpha_, conductor, xi,
                                                   fresnel_result, result);

    fresnel_result *= weight_;

    result.reflection *= n_dot_wi * weight_;
}

}  // namespace scene::material::metallic_paint
