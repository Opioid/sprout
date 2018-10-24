#include "substitute_coating_material.inl"
#include "base/math/vector4.inl"
#include "scene/material/coating/coating.inl"
#include "scene/material/collision_coefficients.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "substitute_coating_sample.inl"

namespace scene::material::substitute {

Material_clearcoat::Material_clearcoat(Sampler_settings const& sampler_settings,
                                       bool                    two_sided) noexcept
    : Material_coating<Clearcoat_data>(sampler_settings, two_sided) {}

material::Sample const& Material_clearcoat::sample(float3 const& wo, Renderstate const& rs,
                                                   Filter filter, sampler::Sampler& /*sampler*/,
                                                   Worker const& worker, uint32_t depth) const
    noexcept {
    auto& sample = worker.sample<Sample_clearcoat>(depth);

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    float thickness;
    float weight;
    if (coating_thickness_map_.is_valid()) {
        float const relative_thickness = coating_thickness_map_.sample_1(sampler, rs.uv);

        thickness = coating_.thickness * relative_thickness;
        weight    = relative_thickness > 0.1f ? 1.f : relative_thickness;
    } else {
        thickness = coating_.thickness;
        weight    = 1.f;
    }

    float const coating_ior = math::lerp(rs.ior, coating_.ior, weight);

    set_sample(wo, rs, coating_ior, sampler, sample);

    set_coating_basis(wo, rs, sampler, sample);

    sample.coating_.set(coating_.absorption_coefficient, thickness, coating_ior,
                        fresnel::schlick_f0(coating_ior, rs.ior), coating_.alpha, weight);

    return sample;
}

void Material_clearcoat::set_coating_attenuation(float3 const& absorption_color,
                                                 float         distance) noexcept {
    coating_.absorption_coefficient = extinction_coefficient(absorption_color, distance);
}

void Material_clearcoat::set_coating_ior(float ior) noexcept {
    coating_.ior = ior;
}

void Material_clearcoat::set_coating_roughness(float roughness) noexcept {
    float const r = ggx::clamp_roughness(roughness);

    coating_.alpha = r * r;
}

size_t Material_clearcoat::sample_size() noexcept {
    return sizeof(Sample_clearcoat);
}

Material_thinfilm::Material_thinfilm(Sampler_settings const& sampler_settings,
                                     bool                    two_sided) noexcept
    : Material_coating<Thinfilm_data>(sampler_settings, two_sided) {}

material::Sample const& Material_thinfilm::sample(float3 const& wo, Renderstate const& rs,
                                                  Filter filter, sampler::Sampler& /*sampler*/,
                                                  Worker const& worker, uint32_t depth) const
    noexcept {
    auto& sample = worker.sample<Sample_thinfilm>(depth);

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, coating_.ior, sampler, sample);

    set_coating_basis(wo, rs, sampler, sample);

    sample.coating_.set(coating_.ior, ior_, coating_.alpha, coating_.thickness);

    return sample;
}

void Material_thinfilm::set_thinfilm(float ior, float roughness, float thickness) noexcept {
    coating_.ior = ior;

    float const r     = ggx::clamp_roughness(roughness);
    float const alpha = r * r;

    coating_.alpha     = alpha;
    coating_.thickness = thickness;
}

size_t Material_thinfilm::sample_size() noexcept {
    return sizeof(Sample_thinfilm);
}

}  // namespace scene::material::substitute
