#include "volumetric_material.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/null/null_sample.hpp"
#include "scene/scene_ray.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "volumetric_sample.hpp"

namespace scene::material::volumetric {

Material::Material(Sampler_settings const& sampler_settings) noexcept
    : material::Material(sampler_settings, true) {}

Material::~Material() noexcept {}

material::Sample const& Material::sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                         Filter /*filter*/, sampler::Sampler& /*sampler*/,
                                         Worker const& worker, uint32_t sample_level) const
    noexcept {
    if (rs.subsurface) {
        auto& sample = worker.sample<Sample>(sample_level);

        sample.set_basis(rs.geo_n, wo);

        float const gs = van_de_hulst_anisotropy(ray.depth);

        sample.set(gs);

        return sample;
    }

    auto& sample = worker.sample<null::Sample>(sample_level);

    sample.set_basis(rs.geo_n, wo);

    return sample;
}

float Material::ior() const noexcept {
    return 1.f;
}

CM Material::control_medium() const noexcept {
    return cm_;
}

float Material::similarity_relation_scale(uint32_t depth) const noexcept {
    float const gs = van_de_hulst_anisotropy(depth);

    return van_de_hulst(anisotropy_, gs);
}

float3 Material::evaluate_radiance(float3 const& /*wi*/, float2 /*uv*/, float /*area*/,
                                   Filter /*filter*/, Worker const& /*worker*/) const noexcept {
    return cc_.a * emission_;
}

float3 Material::average_radiance(float /*area_or_volume*/) const noexcept {
    return cc_.a * emission_;
}

void Material::set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
                               float distance) noexcept {
    cc_ = attenuation(absorption_color, scattering_color, distance);

    cm_ = CM(cc_);
}

void Material::set_emission(float3 const& emission) noexcept {
    emission_ = emission;
}

void Material::set_anisotropy(float anisotropy) noexcept {
    anisotropy_ = std::clamp(anisotropy, -0.999f, 0.999f);
}

size_t Material::sample_size() noexcept {
    return sizeof(Sample);
}

void Material::set_similarity_relation_range(uint32_t low, uint32_t high) {
    SR_low       = low;
    SR_high      = high;
    SR_inv_range = 1.f / static_cast<float>(high - low);
}

uint32_t Material::SR_low  = 16;
uint32_t Material::SR_high = 64;

float Material::SR_inv_range = 1.f / static_cast<float>(Material::SR_high - Material::SR_low);

float Material::van_de_hulst_anisotropy(uint32_t depth) const noexcept {
    if (depth < SR_low) {
        return anisotropy_;
    } else if (depth < SR_high) {
        float const towards_zero = SR_inv_range * static_cast<float>(depth - SR_low);

        return math::lerp(anisotropy_, 0.f, towards_zero);
    }

    return 0.f;
}

}  // namespace scene::material::volumetric
