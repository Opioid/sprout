#include "volumetric_material.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/null/null_sample.hpp"
#include "scene/scene_ray.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "volumetric_sample.hpp"

namespace scene::material::volumetric {

Material::Material(Sampler_settings const& sampler_settings)
    : material::Material(sampler_settings, true) {
    ior_ = 1.f;
    properties_.set(Property::Scattering_volume);
}

Material::~Material() = default;

material::Sample const& Material::sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                         Filter /*filter*/, Sampler& /*sampler*/,
                                         Worker& worker) const {
    if (rs.subsurface) {
        auto& sample = worker.sample<Sample>();

        sample.set_basis(rs.geo_n, rs.n, wo);

        float const gs = van_de_hulst_anisotropy(ray.depth);

        sample.set(gs);

        return sample;
    }

    auto& sample = worker.sample<null::Sample>();

    sample.set_basis(rs.geo_n, rs.n, wo);

    return sample;
}

float3 Material::average_radiance(float /*area_or_volume*/, Scene const& /*scene*/) const {
    return cc_.a * emission_;
}

void Material::set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
                               float distance) {
    if (any_greater_zero(scattering_color)) {
        cc_ = attenuation(absorption_color, scattering_color, distance);
    } else {
        cc_ = {attenuation_coefficient(absorption_color, distance), float3(0.f)};
    }

    cm_ = CM(cc_);

    attenuation_distance_ = distance;
}

void Material::set_emission(float3 const& emission) {
    emission_ = emission;
}

size_t Material::sample_size() {
    return sizeof(Sample);
}

}  // namespace scene::material::volumetric
