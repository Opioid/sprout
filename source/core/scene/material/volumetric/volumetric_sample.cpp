#include "volumetric_sample.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"

namespace scene::material::volumetric {

static inline float phase_hg(float cos_theta, float g) noexcept {
    float const gg    = g * g;
    float const denom = 1.f + gg + 2.f * g * cos_theta;
    return (1.f / (4.f * math::Pi)) * (1.f - gg) / (denom * std::sqrt(denom));
}

const material::Layer& Sample::base_layer() const noexcept {
    return layer_;
}

bxdf::Result Sample::evaluate(float3 const& wi) const noexcept {
    float const phase = Sample::phase(wo_, wi);

    return {float3(phase), phase};
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    float2 const r2 = sampler.generate_sample_2D();

    float3      dir;
    float const phase = sample(wo_, r2, dir);

    result.reflection = float3(phase);
    result.wi         = dir;
    result.pdf        = phase;
    result.wavelength = 0.f;
    result.type.clear(bxdf::Type::Diffuse_reflection);
}

bool Sample::is_translucent() const noexcept {
    return true;
}

void Sample::set(float anisotropy) {
    anisotropy_ = anisotropy;
}

float Sample::phase(float3 const& wo, float3 const& wi) const noexcept {
    float const g = anisotropy_;
    return phase_hg(math::dot(wo, wi), g);
    //	float const k = 1.55f * g - (0.55f * g) * (g * g);
    //	return phase_schlick(math::dot(wo, wi), k);
}

float Sample::sample(float3 const& wo, float2 r2, float3& wi) const noexcept {
    float const g = anisotropy_;

    float cos_theta;
    if (std::abs(g) < 0.001f) {
        cos_theta = 1.f - 2.f * r2[0];
    } else {
        float const gg  = g * g;
        float const sqr = (1.f - gg) / (1.f - g + 2.f * g * r2[0]);

        cos_theta = (1.f + gg - sqr * sqr) / (2.f * g);
    }

    float const sin_theta = std::sqrt(std::max(0.f, 1.f - cos_theta * cos_theta));
    float const phi       = r2[1] * (2.f * math::Pi);

    auto const [t, b] = math::orthonormal_basis(wo);

    wi = math::sphere_direction(sin_theta, cos_theta, phi, t, b, -wo);

    return phase_hg(-cos_theta, g);
}

}  // namespace scene::material::volumetric
