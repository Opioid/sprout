#include "volumetric_material.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/material_attenuation.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/null/null_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "volumetric_sample.hpp"

namespace scene::material::volumetric {

Material::Material(Sampler_settings const& sampler_settings)
    : material::Material(sampler_settings, true) {}

Material::~Material() {}

material::Sample const& Material::sample(float3 const& wo, Renderstate const& rs,
                                         Sampler_filter /*filter*/, sampler::Sampler& /*sampler*/,
                                         Worker const& worker, uint32_t depth) const {
    if (rs.subsurface) {
        auto& sample = worker.sample<Sample>(depth);

        sample.set_basis(rs.geo_n, wo);

        sample.set(anisotropy_);

        return sample;
    }

    auto& sample = worker.sample<null::Sample>(depth);

    sample.set_basis(rs.geo_n, wo);

    return sample;
}

float Material::ior() const {
    return 1.f;
}

CM Material::control_medium() const {
    return cm_;
}

void Material::set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
                               float distance) {
    attenuation(absorption_color, scattering_color, distance, cc_.a, cc_.s);

    cm_ = CM(cc_);
}

void Material::set_anisotropy(float anisotropy) {
    anisotropy_ = std::clamp(anisotropy, -0.999f, 0.999f);
}

size_t Material::sample_size() {
    return sizeof(Sample);
}

}  // namespace scene::material::volumetric
