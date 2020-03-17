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
    : material::Material(sampler_settings, true), is_scattering_(true) {
    ior_ = 1.f;
}

Material::~Material() = default;

material::Sample const& Material::sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                         Filter /*filter*/, Sampler& /*sampler*/,
                                         Worker& worker) const {
    if (rs.subsurface) {
        auto& sample = worker.sample<Sample>();

        sample.set_basis(rs.geo_n, wo);

        float const gs = van_de_hulst_anisotropy(ray.depth);

        sample.set(gs);

        return sample;
    }

    auto& sample = worker.sample<null::Sample>();

    sample.set_basis(rs.geo_n, wo);

    return sample;
}

CM Material::control_medium() const {
    return cm_;
}

float Material::similarity_relation_scale(uint32_t depth) const {
    float const gs = van_de_hulst_anisotropy(depth);

    return van_de_hulst(anisotropy_, gs);
}

bool Material::is_scattering_volume() const {
    return is_scattering_;
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

    distance_ = distance;
}

void Material::set_emission(float3 const& emission) {
    emission_ = emission;
}

void Material::set_anisotropy(float anisotropy) {
    anisotropy_ = std::clamp(anisotropy, -0.999f, 0.999f);
}

size_t Material::sample_size() {
    return sizeof(Sample);
}

void Material::set_similarity_relation_range(uint32_t low, uint32_t high) {
    SR_low       = low;
    SR_high      = high;
    SR_inv_range = 1.f / float(high - low);
}

uint32_t Material::SR_low  = 16;
uint32_t Material::SR_high = 64;

float Material::SR_inv_range = 1.f / float(Material::SR_high - Material::SR_low);

float Material::van_de_hulst_anisotropy(uint32_t depth) const {
    if (depth < SR_low) {
        return anisotropy_;
    }

    if (depth < SR_high) {
        float const towards_zero = SR_inv_range * float(depth - SR_low);

        return lerp(anisotropy_, 0.f, towards_zero);
    }

    return 0.f;
}

}  // namespace scene::material::volumetric
