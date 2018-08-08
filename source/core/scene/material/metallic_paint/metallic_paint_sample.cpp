#include "metallic_paint_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/coating/coating.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"

namespace scene::material::metallic_paint {

const material::Sample::Layer& Sample::base_layer() const noexcept {
    return base_;
}

bxdf::Result Sample::evaluate(float3 const& wi) const noexcept {
    if (!same_hemisphere(wo_)) {
        return {float3::identity(), 0.f};
    }

    float3 const h = math::normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    auto const coating = coating_.evaluate(wi, wo_, h, wo_dot_h, 1.f);

    float3     flakes_fresnel;
    auto const flakes = flakes_.evaluate(wi, wo_, h, wo_dot_h, flakes_fresnel);

    auto const base = base_.evaluate(wi, wo_, h, wo_dot_h);

    float3 const bottom = (1.f - flakes_fresnel) * base.reflection + flakes.reflection;

    float const pdf = (coating.pdf + flakes.pdf + base.pdf) / 3.f;

    return {coating.reflection + coating.attenuation * bottom, pdf};
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float p = sampler.generate_sample_1D();

    if (p < 0.4f) {
        float3 coating_attenuation;
        coating_.sample(wo_, 1.f, sampler, coating_attenuation, result);

        float3     flakes_fresnel;
        auto const flakes = flakes_.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
                                             flakes_fresnel);

        auto const base = base_.evaluate(result.wi, wo_, result.h, result.h_dot_wi);

        float3 const bottom = (1.f - flakes_fresnel) * base.reflection + flakes.reflection;

        result.reflection = result.reflection + coating_attenuation * bottom;
        result.pdf        = (result.pdf + base.pdf + flakes.pdf) / 3.f;
    } else if (p < 0.7f) {
        base_.sample(wo_, sampler, result);

        auto const coating = coating_.evaluate(result.wi, wo_, result.h, result.h_dot_wi, 1.f);

        float3     flakes_fresnel;
        auto const flakes = flakes_.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
                                             flakes_fresnel);

        float3 const bottom = (1.f - flakes_fresnel) * result.reflection + flakes.reflection;

        result.reflection = coating.reflection + coating.attenuation * bottom;
        result.pdf        = (result.pdf + coating.pdf + flakes.pdf) / 3.f;
    } else {
        float3 flakes_fresnel;
        flakes_.sample(wo_, sampler, flakes_fresnel, result);

        auto const coating = coating_.evaluate(result.wi, wo_, result.h, result.h_dot_wi, 1.f);

        auto const base = base_.evaluate(result.wi, wo_, result.h, result.h_dot_wi);

        float3 const bottom = (1.f - flakes_fresnel) * base.reflection + result.reflection;

        result.reflection = coating.reflection + coating.attenuation * bottom;
        result.pdf        = (result.pdf + base.pdf + coating.pdf) / 3.f;
    }

    result.wavelength = 0.f;
}

void Sample::Base_layer::set(float3 const& color_a, float3 const& color_b, float alpha,
                             float alpha2) noexcept {
    color_a_ = color_a;
    color_b_ = color_b;
    alpha_   = alpha;
    alpha2_  = alpha2;
}

bxdf::Result Sample::Base_layer::evaluate(float3 const& wi, float3 const& wo, float3 const& h,
                                          float wo_dot_h) const noexcept {
    float const n_dot_wi = clamp_n_dot(wi);
    float const n_dot_wo = clamp_abs_n_dot(wo);

    float f = n_dot_wo;

    float3 color = math::lerp(color_b_, color_a_, f);

    float const n_dot_h = math::saturate(math::dot(n_, h));

    fresnel::Schlick const fresnel(color);
    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, *this,
                                                fresnel);

    return {n_dot_wi * ggx.reflection, ggx.pdf};
}

void Sample::Base_layer::sample(float3 const& wo, sampler::Sampler& sampler,
                                bxdf::Sample& result) const noexcept {
    float const n_dot_wo = clamp_abs_n_dot(wo);

    float f = n_dot_wo;

    float3 color = math::lerp(color_b_, color_a_, f);

    fresnel::Schlick const fresnel(color);
    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, fresnel, sampler, result);
    result.reflection *= n_dot_wi;
}

void Sample::Flakes_layer::set(float3 const& ior, float3 const& absorption, float alpha,
                               float alpha2, float weight) noexcept {
    ior_        = ior;
    absorption_ = absorption;
    alpha_      = alpha;
    alpha2_     = alpha2;
    weight_     = weight;
}

bxdf::Result Sample::Flakes_layer::evaluate(float3 const& wi, float3 const& wo, float3 const& h,
                                            float wo_dot_h, float3& fresnel_result) const noexcept {
    float const n_dot_wi = clamp_n_dot(wi);
    float const n_dot_wo = clamp_abs_n_dot(wo);

    float const n_dot_h = math::saturate(math::dot(n_, h));

    const fresnel::Conductor_weighted conductor({ior_, absorption_}, weight_);
    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, *this,
                                                conductor, fresnel_result);

    return {n_dot_wi * ggx.reflection, ggx.pdf};
}

void Sample::Flakes_layer::sample(float3 const& wo, sampler::Sampler& sampler,
                                  float3& fresnel_result, bxdf::Sample& result) const noexcept {
    float const n_dot_wo = clamp_abs_n_dot(wo);

    const fresnel::Conductor_weighted conductor({ior_, absorption_}, weight_);
    float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, conductor, sampler,
                                                   fresnel_result, result);
    result.reflection *= n_dot_wi;
}

}  // namespace scene::material::metallic_paint
