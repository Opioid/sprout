#ifndef SU_CORE_SCENE_MATERIAL_MATERIAL_INL
#define SU_CORE_SCENE_MATERIAL_MATERIAL_INL

#include "base/math/sampling.inl"
#include "base/math/vector2.inl"
#include "collision_coefficients.inl"
#include "fresnel/fresnel.inl"
#include "image/texture/texture.inl"
#include "image/texture/texture_sampler.hpp"
#include "material.hpp"
#include "scene/scene_worker.inl"

namespace scene::material {

static inline float phase_hg(float cos_theta, float g) {
    float const gg    = g * g;
    float const denom = 1.f + gg + 2.f * g * cos_theta;
    return (1.f / (4.f * Pi)) * (1.f - gg) / (denom * std::sqrt(denom));
}

static inline float4 sample_phase(float3_p wo, float2 r2, float g) {
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

inline float Material::opacity(float2 uv, Filter filter, Worker const& worker) const {
    if (mask_.is_valid()) {
        auto const& sampler = worker.sampler_2D(sampler_key_, filter);

        return sampler.sample_1(mask_, uv, worker.scene());
    }

    return 1.f;
}

inline float Material::border(float3_p wi, float3_p n) const {
    float const f0 = fresnel::schlick_f0(ior_, 1.f);

    float const n_dot_wi = std::max(dot(n, wi), 0.f);

    float const f = 1.f - fresnel::schlick(n_dot_wi, f0);

    return f;
}

inline CC Material::collision_coefficients() const {
    return cc_;
}

inline CC Material::collision_coefficients(float2 uv, Filter filter, Worker const& worker) const {
    if (color_map_.is_valid()) {
        auto const&  sampler = worker.sampler_2D(sampler_key(), filter);
        float3 const color   = sampler.sample_3(color_map_, uv, worker.scene());

        return scattering(cc_.a, color, volumetric_anisotropy_);
    }

    return cc_;
}

inline uint32_t Material::sampler_key() const {
    return sampler_key_;
}

inline bool Material::is_two_sided() const {
    return properties_.is(Property::Two_sided);
}

inline bool Material::is_masked() const {
    return mask_.is_valid();
}

inline bool Material::is_animated() const {
    return properties_.is(Property::Animated);
}

inline bool Material::is_caustic() const {
    return properties_.is(Property::Caustic);
}

inline bool Material::has_tinted_shadow() const {
    return properties_.is(Property::Tinted_shadow);
}

inline bool Material::has_emission_map() const {
    return properties_.is(Property::Emission_map);
}

inline bool Material::is_pure_emissive() const {
    return properties_.is(Property::Pure_emissive);
}

inline bool Material::is_scattering_volume() const {
    return properties_.is(Property::Scattering_volume);
}

inline bool Material::is_textured_volume() const {
    return properties_.is(Property::Textured_volume);
}

inline bool Material::is_heterogeneous_volume() const {
    return properties_.is(Property::Heterogeneous_volume);
}

inline float Material::ior() const {
    return ior_;
}

inline float Material::phase(float3_p wo, float3_p wi) const {
    float const g = volumetric_anisotropy_;
    return phase_hg(dot(wo, wi), g);
}

inline float4 Material::sample_phase(float3_p wo, float2 r2) const {
    float const g = volumetric_anisotropy_;
    return material::sample_phase(wo, r2, g);
}

inline Material::Radiance_sample::Radiance_sample(float2 uv, float pdf)
    : uvw{uv[0], uv[1], 0.f, pdf} {}

inline Material::Radiance_sample::Radiance_sample(float3_p uvw, float pdf)
    : uvw{uvw[0], uvw[1], uvw[2], pdf} {}

inline float Material::Radiance_sample::pdf() const {
    return uvw[3];
}

}  // namespace scene::material

#endif
