#include "volumetric_sample.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"

namespace scene::material::volumetric {

static inline float phase_hg(float cos_theta, float g) {
    float const gg    = g * g;
    float const denom = 1.f + gg + 2.f * g * cos_theta;
    return (1.f / (4.f * Pi)) * (1.f - gg) / (denom * std::sqrt(denom));
}

float3 const& Sample::base_shading_normal() const {
    return geo_n_;
}

bxdf::Result Sample::evaluate_f(float3 const& wi) const {
    float const phase = Sample::phase(wo_, wi);

    return {float3(phase), phase};
}

bxdf::Result Sample::evaluate_b(float3 const& wi) const {
    float const phase = Sample::phase(wo_, wi);

    return {float3(phase), phase};
}

void Sample::sample(Sampler& sampler, bxdf::Sample& result) const {
    float2 const r2 = sampler.generate_sample_2D();

    float4 const ps = sample(wo_, r2);

    result.reflection = float3(ps[3]);
    result.wi         = ps.xyz();
    result.pdf        = ps[3];
    result.wavelength = 0.f;
    result.type.clear(bxdf::Type::Diffuse_reflection);
}

bool Sample::is_translucent() const {
    return true;
}

void Sample::set(float anisotropy) {
    anisotropy_ = anisotropy;
}

float Sample::phase(float3 const& wo, float3 const& wi) const {
    float const g = anisotropy_;
    return phase_hg(dot(wo, wi), g);
    //	float const k = 1.55f * g - (0.55f * g) * (g * g);
    //	return phase_schlick(dot(wo, wi), k);
}

float4 Sample::sample(float3 const& wo, float2 r2) const {
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
    float const phi       = r2[1] * (2.f * Pi);

    auto const [t, b] = orthonormal_basis(wo);

    float3 const wi = math::sphere_direction(sin_theta, cos_theta, phi, t, b, -wo);

    return float4(wi, phase_hg(-cos_theta, g));
}

}  // namespace scene::material::volumetric
