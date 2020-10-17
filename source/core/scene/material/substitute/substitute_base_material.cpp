#include "substitute_base_material.hpp"
#include "base/math/vector4.inl"
#include "image/texture/texture.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene_worker.inl"
#include "substitute_sample.hpp"

namespace scene::material::substitute {

Material_base::Material_base(Sampler_settings const& sampler_settings, bool two_sided)
    : material::Material(sampler_settings, two_sided) {}

void Material_base::commit(Threads& /*threads*/, Scene const& scene) {
    properties_.set(Property::Caustic, !surface_map_.is_valid() && alpha_ <= ggx::Min_alpha);

    if (emission_map_.is_valid()) {
        average_emission_ = emission_map_.texture(scene).average_3();
    }
}

float3 Material_base::evaluate_radiance(float3 const& /*wi*/, float3 const& uvw, float /*extent*/,
                                        Filter filter, Worker const& worker) const {
    if (emission_map_.is_valid()) {
        auto const& sampler = worker.sampler_2D(sampler_key(), filter);
        return emission_factor_ * emission_map_.sample_3(worker, sampler, uvw.xy());
    }

    return float3(0.f);
}

float3 Material_base::average_radiance(float /*area*/) const {
    if (emission_map_.is_valid()) {
        return emission_factor_ * average_emission_;
    }

    return float3(0.f);
}

void Material_base::set_color_map(Texture_adapter const& color_map) {
    color_map_ = color_map;
}

void Material_base::set_normal_map(Texture_adapter const& normal_map) {
    normal_map_ = normal_map;
}

void Material_base::set_surface_map(Texture_adapter const& surface_map) {
    surface_map_ = surface_map;
}

void Material_base::set_emission_map(Texture_adapter const& emission_map) {
    emission_map_ = emission_map;

    properties_.set(Property::Emission_map, emission_map.is_valid());
}

void Material_base::set_color(float3 const& color) {
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
