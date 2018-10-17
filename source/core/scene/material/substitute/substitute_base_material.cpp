#include "substitute_base_material.hpp"
#include "base/math/vector4.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene_worker.inl"
#include "substitute_sample.hpp"

namespace scene::material::substitute {

Material_base::Material_base(Sampler_settings const& sampler_settings, bool two_sided) noexcept
    : material::Material(sampler_settings, two_sided) {}

float3 Material_base::evaluate_radiance(float3 const& /*wi*/, float2   uv, float /*area*/,
                                        uint64_t /*time*/, Sampler_filter filter,
                                        Worker const& worker) const noexcept {
    if (emission_map_.is_valid()) {
        // For some reason Clang needs this to find inherited Material::sampler_key_
        auto const& sampler = worker.sampler_2D(sampler_key(), filter);
        return emission_factor_ * emission_map_.sample_3(sampler, uv);
    } else {
        return float3(0.f);
    }
}

float3 Material_base::average_radiance(float /*area*/) const noexcept {
    if (emission_map_.is_valid()) {
        return emission_factor_ * emission_map_.texture().average_3();
    } else {
        return float3(0.f);
    }
}

bool Material_base::has_emission_map() const noexcept {
    return emission_map_.is_valid();
}

float Material_base::ior() const noexcept {
    return ior_;
}

void Material_base::set_ior(float ior) noexcept {
    ior_ = ior;
}

bool Material_base::is_caustic() const noexcept {
    return !surface_map_.is_valid() && alpha_ <= ggx::Min_alpha;
}

void Material_base::set_color_map(Texture_adapter const& color_map) noexcept {
    color_map_ = color_map;
}

void Material_base::set_normal_map(Texture_adapter const& normal_map) noexcept {
    normal_map_ = normal_map;
}

void Material_base::set_surface_map(Texture_adapter const& surface_map) noexcept {
    surface_map_ = surface_map;
}

void Material_base::set_emission_map(Texture_adapter const& emission_map) noexcept {
    emission_map_ = emission_map;
}

void Material_base::set_color(float3 const& color) noexcept {
    color_ = color;
}

void Material_base::set_roughness(float roughness) noexcept {
    const float r = ggx::clamp_roughness(roughness);

    alpha_ = r * r;
}

void Material_base::set_metallic(float metallic) noexcept {
    metallic_ = metallic;
}

void Material_base::set_emission_factor(float emission_factor) noexcept {
    emission_factor_ = emission_factor;
}

}  // namespace scene::material::substitute
