#include "metallic_paint_material.hpp"
#include "base/math/vector4.inl"
#include "image/texture/texture_adapter.inl"
#include "metallic_paint_sample.hpp"
#include "scene/material/coating/coating.inl"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::metallic_paint {

Material::Material(Sampler_settings const& sampler_settings, bool two_sided) noexcept
    : material::Material(sampler_settings, two_sided) {}

material::Sample const& Material::sample(float3 const&      wo, Ray const& /*ray*/,
                                         Renderstate const& rs, Filter                filter,
                                         sampler::Sampler& /*sampler*/, Worker const& worker) const
    noexcept {
    auto& sample = worker.sample<Sample>();

    sample.set_basis(rs.geo_n, wo);

    sample.base_.set_tangent_frame(rs.t, rs.b, rs.n);

    sample.coating_.set_tangent_frame(rs.t, rs.b, rs.n);

    if (flakes_normal_map_.is_valid()) {
        auto const& sampler = worker.sampler_2D(sampler_key(), Filter::Nearest);

        float3 const n = sample_normal(wo, rs, flakes_normal_map_, sampler, worker);

        sample.flakes_.set_tangent_frame(n);
    } else {
        sample.flakes_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    sample.base_.set(color_a_, color_b_, alpha_);

    float flakes_weight;
    if (flakes_mask_.is_valid()) {
        auto const& sampler = worker.sampler_2D(sampler_key(), filter);

        flakes_weight = flakes_mask_.sample_1(worker, sampler, rs.uv);
    } else {
        flakes_weight = 1.f;
    }

    float const flakes_alpha = rs.avoid_caustics ? 1.f : flakes_alpha_;

    sample.flakes_.set(flakes_ior_, flakes_absorption_, flakes_alpha, flakes_weight);

    sample.coating_.set(coating_.absorption_coefficient, coating_.thickness, coating_.ior,
                        fresnel::schlick_f0(coating_.ior, rs.ior), coating_.alpha, 1.f);

    sample.avoid_caustics_ = rs.avoid_caustics;

    return sample;
}

float Material::ior() const noexcept {
    return ior_;
}

size_t Material::num_bytes() const noexcept {
    return sizeof(*this);
}

void Material::set_color(float3 const& a, float3 const& b) noexcept {
    color_a_ = a;
    color_b_ = b;
}

void Material::set_roughness(float roughness) noexcept {
    float const r = ggx::clamp_roughness(roughness);

    alpha_ = r * r;
}

void Material::set_flakes_mask(Texture_adapter const& mask) noexcept {
    flakes_mask_ = mask;
}

void Material::set_flakes_normal_map(Texture_adapter const& normal_map) noexcept {
    flakes_normal_map_ = normal_map;
}

void Material::set_flakes_ior(float3 const& ior) noexcept {
    flakes_ior_ = ior;
}

void Material::set_flakes_absorption(float3 const& absorption) noexcept {
    flakes_absorption_ = absorption;
}

void Material::set_flakes_roughness(float roughness) noexcept {
    float const r = ggx::clamp_roughness(roughness);

    flakes_alpha_ = r * r;
}

void Material::set_coating_thickness(float thickness) noexcept {
    coating_.thickness = thickness;
}

void Material::set_coating_attenuation(float3 const& absorption_color, float distance) noexcept {
    coating_.absorption_coefficient = extinction_coefficient(absorption_color, distance);
}

void Material::set_coating_ior(float ior) noexcept {
    coating_.ior = ior;
}

void Material::set_coating_roughness(float roughness) noexcept {
    float const r = ggx::clamp_roughness(roughness);

    coating_.alpha = r * r;
}

size_t Material::sample_size() noexcept {
    return sizeof(Sample);
}

}  // namespace scene::material::metallic_paint
