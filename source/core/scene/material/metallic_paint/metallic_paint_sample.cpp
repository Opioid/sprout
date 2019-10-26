#include "metallic_paint_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/coating/coating.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"

namespace scene::material::metallic_paint {

float3 const& Sample::base_shading_normal() const noexcept {
    return base_.n_;
}

float3 Sample::base_tangent_to_world(float3 const& v) const noexcept {
    return base_.tangent_to_world(v);
}

bxdf::Result Sample::evaluate_f(float3 const& wi, bool) const noexcept {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    auto const coating = coating_.evaluate_f(wi, wo_, h, wo_dot_h, avoid_caustics_);

    float3     flakes_fresnel;
    auto const flakes = flakes_.evaluate<true>(wi, wo_, h, wo_dot_h, flakes_fresnel);

    auto const base = base_.evaluate<true>(wi, wo_, h, wo_dot_h);

    float3 const bottom = (1.f - flakes_fresnel) * base.reflection + flakes.reflection;

    float const pdf = (coating.pdf + flakes.pdf + base.pdf) / 3.f;

    return {coating.reflection + coating.attenuation * bottom, pdf};
}

bxdf::Result Sample::evaluate_b(float3 const& wi, bool) const noexcept {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    auto const coating = coating_.evaluate_b(wi, wo_, h, wo_dot_h, avoid_caustics_);

    float3     flakes_fresnel;
    auto const flakes = flakes_.evaluate<false>(wi, wo_, h, wo_dot_h, flakes_fresnel);

    auto const base = base_.evaluate<false>(wi, wo_, h, wo_dot_h);

    float3 const bottom = (1.f - flakes_fresnel) * base.reflection + flakes.reflection;

    float const pdf = (coating.pdf + flakes.pdf + base.pdf) / 3.f;

    return {coating.reflection + coating.attenuation * bottom, pdf};
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const p = sampler.generate_sample_1D();

    if (p < 0.4f) {
        float3 coating_attenuation;
        coating_.sample(wo_, sampler, coating_attenuation, result);

        float3     flakes_fresnel;
        auto const flakes = flakes_.evaluate<true>(result.wi, wo_, result.h, result.h_dot_wi,
                                                   flakes_fresnel);

        auto const base = base_.evaluate<true>(result.wi, wo_, result.h, result.h_dot_wi);

        float3 const bottom = (1.f - flakes_fresnel) * base.reflection + flakes.reflection;

        result.reflection = result.reflection + coating_attenuation * bottom;
        result.pdf        = (result.pdf + base.pdf + flakes.pdf) * 0.4f;
    } else if (p < 0.8f) {
        base_.sample(wo_, sampler, result);

        auto const coating = coating_.evaluate_f(result.wi, wo_, result.h, result.h_dot_wi,
                                                 avoid_caustics_);

        float3     flakes_fresnel;
        auto const flakes = flakes_.evaluate<true>(result.wi, wo_, result.h, result.h_dot_wi,
                                                   flakes_fresnel);

        float3 const bottom = (1.f - flakes_fresnel) * result.reflection + flakes.reflection;

        result.reflection = coating.reflection + coating.attenuation * bottom;
        result.pdf        = (result.pdf + coating.pdf + flakes.pdf) * 0.4f;
    } else {
        float3 flakes_fresnel;
        flakes_.sample(wo_, sampler, flakes_fresnel, result);

        auto const coating = coating_.evaluate_f(result.wi, wo_, result.h, result.h_dot_wi,
                                                 avoid_caustics_);

        auto const base = base_.evaluate<true>(result.wi, wo_, result.h, result.h_dot_wi);

        float3 const bottom = (1.f - flakes_fresnel) * base.reflection + result.reflection;

        result.reflection = coating.reflection + coating.attenuation * bottom;
        result.pdf        = (result.pdf + base.pdf + coating.pdf) * 0.2f;
    }
}

void Sample::Base_layer::set(float3 const& color_a, float3 const& color_b, float alpha) noexcept {
    color_a_ = color_a;
    color_b_ = color_b;
    alpha_   = alpha;
}

template <bool Forward>
bxdf::Result Sample::Base_layer::evaluate(float3 const& wi, float3 const& wo, float3 const& h,
                                          float wo_dot_h) const noexcept {
    float const n_dot_wi = clamp_n_dot(wi);
    float const n_dot_wo = clamp_abs_n_dot(wo);

    float const f = n_dot_wo;

    float3 const color = lerp(color_b_, color_a_, f);

    float const n_dot_h = saturate(dot(n_, h));

    fresnel::Schlick const fresnel(color);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_,
                                                fresnel);

    if constexpr (Forward) {
        return {n_dot_wi * ggx.reflection, ggx.pdf};
    } else {
        return ggx;
    }
}

void Sample::Base_layer::sample(float3 const& wo, Sampler& sampler, bxdf::Sample& result) const
    noexcept {
    float const n_dot_wo = clamp_abs_n_dot(wo);

    float const f = n_dot_wo;

    float3 const color = lerp(color_b_, color_a_, f);

    fresnel::Schlick const fresnel(color);

    float2 const xi = sampler.generate_sample_2D();

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, alpha_, fresnel, xi,
                                                   result);
    result.reflection *= n_dot_wi;
}

void Sample::Flakes_layer::set(float3 const& ior, float3 const& absorption, float alpha,
                               float weight) noexcept {
    ior_        = ior;
    absorption_ = absorption;
    alpha_      = alpha;
    weight_     = weight;
}

template <bool Forward>
bxdf::Result Sample::Flakes_layer::evaluate(float3 const& wi, float3 const& wo, float3 const& h,
                                            float wo_dot_h, float3& fresnel_result) const noexcept {
    float const n_dot_wi = clamp_n_dot(wi);
    float const n_dot_wo = clamp_abs_n_dot(wo);

    float const n_dot_h = saturate(dot(n_, h));

    fresnel::Conductor const conductor(ior_, absorption_);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_,
                                                conductor, fresnel_result);

    fresnel_result *= weight_;

    if constexpr (Forward) {
        return {n_dot_wi * weight_ * ggx.reflection, ggx.pdf};
    } else {
        return {weight_ * ggx.reflection, ggx.pdf};
    }
}

void Sample::Flakes_layer::sample(float3 const& wo, Sampler& sampler, float3& fresnel_result,
                                  bxdf::Sample& result) const noexcept {
    float const n_dot_wo = clamp_abs_n_dot(wo);

    fresnel::Conductor const conductor(ior_, absorption_);

    float2 const xi = sampler.generate_sample_2D();

    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, alpha_, conductor, xi,
                                                   fresnel_result, result);

    fresnel_result *= weight_;

    result.reflection *= n_dot_wi * weight_;
}

}  // namespace scene::material::metallic_paint
