#include "substitute_base_material.hpp"
#include "base/math/vector4.inl"
#include "image/texture/texture.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene_worker.inl"
#include "substitute_sample.hpp"

namespace scene::material::substitute {

Material_base::Material_base(Sampler_settings sampler_settings, bool two_sided)
    : material::Material(sampler_settings, two_sided) {}

void Material_base::commit(Threads& /*threads*/, Scene const& scene) {
    properties_.set(Property::Caustic, alpha_ <= ggx::Min_alpha);

    if (emission_map_.is_valid()) {
        average_emission_ = emission_factor_ * emission_map_.average_3(scene);
    } else {
        average_emission_ = emission_factor_ * emission_;
    }
}

float3 Material_base::evaluate_radiance(float3_p /*wi*/, float3_p uvw, float /*extent*/,
                                        Filter filter, Worker& worker) const {
    if (emission_map_.is_valid()) {
        auto const& sampler = worker.sampler_2D(sampler_key(), filter);
        return emission_factor_ * sampler.sample_3(emission_map_, uvw.xy(), worker.scene());
    }

    return average_emission_;
}

float3 Material_base::average_radiance(float /*area*/) const {
    return average_emission_;
}

void Material_base::set_normal_map(Texture const& normal_map) {
    normal_map_ = normal_map;
}

void Material_base::set_surface_map(Texture const& surface_map) {
    surface_map_ = surface_map;
}

void Material_base::set_emission_map(Texture const& emission_map) {
    emission_map_ = emission_map;

    properties_.set(Property::Emission_map, emission_map.is_valid());
}

void Material_base::set_color(float3_p color) {
    color_ = color;
}

void Material_base::set_roughness(float roughness) {
    float const r = ggx::clamp_roughness(roughness);

    alpha_ = r * r;
}

void Material_base::set_metallic(float metallic) {
    metallic_ = metallic;
}

void Material_base::set_emission_factor(float emission_factor) {
    emission_factor_ = emission_factor;
}

}  // namespace scene::material::substitute
