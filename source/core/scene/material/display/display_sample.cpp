#include "display_sample.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"

namespace scene::material::display {

const material::Sample::Layer& Sample::base_layer() const noexcept {
    return layer_;
}

bxdf::Result Sample::evaluate(float3 const& wi) const noexcept {
    if (!same_hemisphere(wo_)) {
        return {float3::identity(), 0.f};
    }

    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);  // layer_.clamp_n_dot(wo_);

    float3 const h = math::normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    float const n_dot_h = math::saturate(math::dot(layer_.n_, h));

    fresnel::Schlick const schlick(layer_.f0_);
    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, layer_,
                                                schlick);

    return {n_dot_wi * ggx.reflection, ggx.pdf};
}

float3 Sample::radiance() const noexcept {
    return layer_.emission_;
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);  // layer_.clamp_n_dot(wo_);

    fresnel::Schlick const schlick(layer_.f0_);
    float const n_dot_wi = ggx::Isotropic::reflect(wo_, n_dot_wo, layer_, schlick, sampler, result);

    result.reflection *= n_dot_wi;

    result.wavelength = 0.f;
}

void Sample::Layer::set(float3 const& radiance, float f0, float roughness) noexcept {
    emission_ = radiance;
    f0_       = float3(f0);

    float const alpha = roughness * roughness;

    alpha_  = alpha;
    alpha2_ = alpha * alpha;
}

}  // namespace scene::material::display
