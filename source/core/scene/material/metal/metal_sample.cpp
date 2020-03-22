#include "metal_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"

namespace scene::material::metal {

bxdf::Result Sample_isotropic::evaluate_f(float3 const& wi) const {
    return evaluate<true>(wi);
}

bxdf::Result Sample_isotropic::evaluate_b(float3 const& wi) const {
    return evaluate<false>(wi);
}

void Sample_isotropic::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    fresnel::Conductor const conductor(ior_, absorption_);

    float2 const xi = sampler.generate_sample_2D();

    float const n_dot_wi = ggx::Isotropic::reflect(wo_, n_dot_wo, layer_, alpha_, conductor, xi,
                                                   result);
    result.reflection *= n_dot_wi;

    result.wavelength = 0.f;
}

void Sample_isotropic::set(float3 const& ior, float3 const& absorption, float alpha,
                           bool avoid_caustics) {
    ior_            = ior;
    absorption_     = absorption;
    alpha_          = alpha;
    avoid_caustics_ = avoid_caustics;
}

template <bool Forward>
bxdf::Result Sample_isotropic::evaluate(float3 const& wi) const {
    if (!same_hemisphere(wo_) || (avoid_caustics_ && alpha_ <= ggx::Min_alpha)) {
        return {float3(0.f), 0.f};
    }

    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    float const n_dot_h = saturate(dot(layer_.n_, h));

    fresnel::Conductor const conductor(ior_, absorption_);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_,
                                                conductor);

    if constexpr (Forward) {
        return {n_dot_wi * ggx.reflection, ggx.pdf()};
    } else {
        return {ggx.reflection, ggx.pdf()};
    }
}

bxdf::Result Sample_anisotropic::evaluate_f(float3 const& wi) const {
    return evaluate<true>(wi);
}

bxdf::Result Sample_anisotropic::evaluate_b(float3 const& wi) const {
    return evaluate<false>(wi);
}

void Sample_anisotropic::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    fresnel::Conductor const conductor(layer_.ior_, layer_.absorption_);

    float2 const xi = sampler.generate_sample_2D();

    float const n_dot_wi = ggx::Anisotropic::reflect(wo_, n_dot_wo, layer_, conductor, xi, result);
    result.reflection *= n_dot_wi;

    result.wavelength = 0.f;
}

template <bool Forward>
bxdf::Result Sample_anisotropic::evaluate(float3 const& wi) const {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    fresnel::Conductor const conductor(layer_.ior_, layer_.absorption_);

    auto const ggx = ggx::Anisotropic::reflection(h, n_dot_wi, n_dot_wo, wo_dot_h, layer_,
                                                  conductor);

    if constexpr (Forward) {
        return {n_dot_wi * ggx.reflection, ggx.pdf()};
    } else {
        return ggx;
    }
}

void Sample_anisotropic::PLayer::set(float3 const& ior, float3 const& absorption,
                                     float2 roughness) {
    ior_        = ior;
    absorption_ = absorption;

    float2 const a = roughness * roughness;
    a_             = a;
    alpha2_        = a * a;
    axy_           = a[0] * a[1];
}

}  // namespace scene::material::metal
