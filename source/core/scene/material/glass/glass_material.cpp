#include "glass_material.hpp"
#include "base/math/vector4.inl"
#include "glass_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::glass {

Glass::Glass(Sampler_settings const& sampler_settings) noexcept
    : Material(sampler_settings, false) {}

material::Sample const& Glass::sample(float3 const& wo, Renderstate const& rs, Filter filter,
                                      sampler::Sampler& /*sampler*/, Worker const& worker,
                                      uint32_t depth) const noexcept {
    auto& sample = worker.sample<Sample>(depth);

    sample.set_basis(rs.geo_n, wo);

    if (normal_map_.is_valid()) {
        auto& sampler = worker.sampler_2D(sampler_key(), filter);

        float3 const n = sample_normal(wo, rs, normal_map_, sampler);
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    sample.set(refraction_color_, ior_, rs.ior);

    return sample;
}

float3 Glass::absorption_coefficient(float2 /*uv*/, Filter /*filter*/,
                                     Worker const& /*worker*/) const noexcept {
    return absorption_coefficient_;
}

float Glass::ior() const noexcept {
    return ior_;
}

bool Glass::is_scattering_volume() const noexcept {
    return false;
}

size_t Glass::num_bytes() const noexcept {
    return sizeof(*this);
}

void Glass::set_normal_map(Texture_adapter const& normal_map) noexcept {
    normal_map_ = normal_map;
}

void Glass::set_refraction_color(float3 const& color) noexcept {
    refraction_color_ = color;
}

void Glass::set_attenuation(float3 const& absorption_color, float distance) noexcept {
    absorption_color_ = absorption_color;

    absorption_coefficient_ = extinction_coefficient(absorption_color, distance);

    attenuation_distance_ = distance;
}

void Glass::set_ior(float ior) noexcept {
    ior_ = ior;
}

bool Glass::is_caustic() const noexcept {
    return true;
}

size_t Glass::sample_size() noexcept {
    return sizeof(Sample);
}

}  // namespace scene::material::glass
