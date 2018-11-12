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

float Material::van_de_hulst_scattering_scale(uint32_t depth) const noexcept {
    float const gs = van_de_hulst_anisotropy(depth);

    return van_de_hulst(anisotropy_, gs);
}

void Material::set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
                               float distance) noexcept {
    cc_ = attenuation(absorption_color, scattering_color, distance);

    cm_ = CM(cc_);
}

void Material::set_anisotropy(float anisotropy) noexcept {
    anisotropy_ = std::clamp(anisotropy, -0.999f, 0.999f);
}

size_t Material::sample_size() noexcept {
    return sizeof(Sample);
}

float Material::van_de_hulst_anisotropy(uint32_t depth) const noexcept {
    static uint32_t constexpr low  = 16;
    static uint32_t constexpr high = 32;

    if (depth < low) {
        return anisotropy_;
    } else if (depth < high) {
        float const towards_zero = static_cast<float>(depth - low) / static_cast<float>(high - low);

        return math::lerp(anisotropy_, 0.f, towards_zero);
    }

    return 0.f;
}

}  // namespace scene::material::volumetric
