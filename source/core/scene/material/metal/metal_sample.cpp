#include "metal_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"

namespace scene::material::metal {

bxdf::Result Sample_isotropic::evaluate_f(float3_p wi) const {
    return evaluate<true>(wi);
}

bxdf::Result Sample_isotropic::evaluate_b(float3_p wi) const {
    return evaluate<false>(wi);
}

void Sample_isotropic::sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    fresnel::Conductor const conductor(ior_, absorption_);

    float2 const xi = sampler.sample_2D(rng);

    float const n_dot_wi = ggx::Iso::reflect(wo_, n_dot_wo, alpha_, conductor, xi, layer_, result);
    result.reflection *= n_dot_wi;

    result.wavelength = 0.f;
}

void Sample_isotropic::set(float3_p ior, float3_p absorption) {
    ior_        = ior;
    absorption_ = absorption;
}

template <bool Forward>
bxdf::Result Sample_isotropic::evaluate(float3_p wi) const {
    if (!same_hemisphere(wo_) || (avoid_caustics() && alpha_ <= ggx::Min_alpha)) {
        return {float3(0.f), 0.f};
    }

    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    float const n_dot_h = saturate(dot(layer_.n_, h));

    fresnel::Conductor const conductor(ior_, absorption_);

    auto const ggx = ggx::Iso::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_, conductor);

    if constexpr (Forward) {
        return {n_dot_wi * ggx.reflection, ggx.pdf()};
    } else {
        return {ggx.reflection, ggx.pdf()};
    }
}

bxdf::Result Sample_anisotropic::evaluate_f(float3_p wi) const {
    return evaluate<true>(wi);
}

bxdf::Result Sample_anisotropic::evaluate_b(float3_p wi) const {
    return evaluate<false>(wi);
}

void Sample_anisotropic::sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    fresnel::Conductor const conductor(ior_, absorption_);

    float2 const xi = sampler.sample_2D(rng);

    float const n_dot_wi = ggx::Aniso::reflect(wo_, n_dot_wo, alpha_, layer_, conductor, xi,
                                               result);
    result.reflection *= n_dot_wi;

    result.wavelength = 0.f;
}

template <bool Forward>
bxdf::Result Sample_anisotropic::evaluate(float3_p wi) const {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    fresnel::Conductor const conductor(ior_, absorption_);

    auto const ggx = ggx::Aniso::reflection(h, n_dot_wi, n_dot_wo, wo_dot_h, alpha_, layer_,
                                            conductor);

    if constexpr (Forward) {
        return {n_dot_wi * ggx.reflection, ggx.pdf()};
    } else {
        return ggx;
    }
}

void Sample_anisotropic::set(float3_p ior, float3_p absorption, float2 alpha) {
    ior_        = ior;
    absorption_ = absorption;
    alpha_      = alpha;
}

}  // namespace scene::material::metal
