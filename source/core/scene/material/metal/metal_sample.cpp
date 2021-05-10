#include "metal_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"

namespace scene::material::metal {

bxdf::Result Sample::evaluate(float3_p wi) const {
    if (!same_hemisphere(wo_) || (avoid_caustics() && alpha_[0] <= ggx::Min_alpha)) {
        return {float3(0.f), 0.f};
    }

    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    fresnel::Conductor const conductor(ior_, absorption_);

    auto const ggx = ggx::Aniso::reflection(wi, wo_, h, n_dot_wi, n_dot_wo, wo_dot_h, alpha_,
                                            conductor, layer_);

    return {n_dot_wi * ggx.reflection, ggx.pdf()};
}

void Sample::sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    fresnel::Conductor const conductor(ior_, absorption_);

    float2 const xi = sampler.sample_2D(rng);

    float const n_dot_wi = ggx::Aniso::reflect(wo_, n_dot_wo, alpha_, conductor, xi, layer_,
                                               result);
    result.reflection *= n_dot_wi;

    result.wavelength = 0.f;
}

void Sample::set(float3_p ior, float3_p absorption) {
    ior_        = ior;
    absorption_ = absorption;
}

}  // namespace scene::material::metal
