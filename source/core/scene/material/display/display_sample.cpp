#include "display_sample.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"

namespace scene::material::display {

bxdf::Result Sample::evaluate_f(float3_p wi) const {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    float const n_dot_h = saturate(dot(layer_.n_, h));

    fresnel::Schlick const schlick(f0_);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_,
                                                schlick);

    return {n_dot_wi * ggx.reflection, ggx.pdf()};
}

bxdf::Result Sample::evaluate_b(float3_p wi) const {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    float const n_dot_h = saturate(dot(layer_.n_, h));

    fresnel::Schlick const schlick(f0_);

    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_,
                                                schlick);

    return {ggx.reflection, ggx.pdf()};
}

void Sample::sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    fresnel::Schlick const schlick(f0_);

    float2 const xi = sampler.sample_2D(rng);

    float const n_dot_wi = ggx::Isotropic::reflect(wo_, n_dot_wo, layer_, alpha_, schlick, xi,
                                                   result);

    result.reflection *= n_dot_wi;

    result.wavelength = 0.f;
}

void Sample::set(float f0) {
    f0_ = f0;
}

}  // namespace scene::material::display
