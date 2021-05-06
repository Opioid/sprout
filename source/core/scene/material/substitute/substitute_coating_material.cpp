#include "substitute_coating_material.inl"
#include "base/math/vector4.inl"
#include "scene/material/coating/coating.inl"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/material.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "substitute_coating_sample.inl"

namespace scene::material::substitute {

Material_clearcoat::Material_clearcoat(Sampler_settings sampler_settings, bool two_sided)
    : Material_coating<Clearcoat_data>(sampler_settings, two_sided) {}

float3 Material_clearcoat::evaluate_radiance(float3_p wi, float3_p n, float3_p uvw,
                                             float /*extent*/, Filter filter,
                                             Worker const& worker) const {
    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    float2 const uv = uvw.xy();

    float3 radiance;

    if (emission_map_.is_valid()) {
        radiance = emission_factor_ * sampler.sample_3(emission_map_, uv, worker.scene());
    } else {
        radiance = average_emission_;
    }

    float thickness;
    if (coating_thickness_map_.is_valid()) {
        float const relative_thickness = sampler.sample_1(coating_thickness_map_, uv,
                                                          worker.scene());

        thickness = coating_.thickness * relative_thickness;
    } else {
        thickness = coating_.thickness;
    }

    coating::Clearcoat clearcoat;

    clearcoat.set(coating_.absorption_coef, thickness, 1.f, 1.f, 1.f, 1.f);

    return clearcoat.attenuation(clamp_abs_dot(wi, n)) * radiance;
}

material::Sample const& Material_clearcoat::sample(float3_p wo, Renderstate const& rs,
                                                   Sampler& /*sampler*/, Worker&   worker) const {
    auto& sample = worker.sample<Sample_clearcoat>();

    auto& sampler = worker.sampler_2D(sampler_key(), rs.filter);

    float thickness;
    float weight;
    if (coating_thickness_map_.is_valid()) {
        float const relative_thickness = sampler.sample_1(coating_thickness_map_, rs.uv,
                                                          worker.scene());

        thickness = coating_.thickness * relative_thickness;
        weight    = relative_thickness > 0.1f ? 1.f : relative_thickness;
    } else {
        thickness = coating_.thickness;
        weight    = 1.f;
    }

    float const coating_ior = lerp(rs.ior(), coating_.ior, weight);

    set_sample(wo, rs, coating_ior, sampler, worker, sample);

    set_coating_basis(wo, rs, sampler, worker, sample);

    sample.coating_.set(coating_.absorption_coef, thickness, coating_ior,
                        fresnel::schlick_f0(coating_ior, rs.ior()), coating_.alpha, weight);

    float const n_dot_wo = sample.coating_.clamp_abs_n_dot(wo);

    sample.set_radiance(sample.radiance() * sample.coating_.attenuation(n_dot_wo));

    return sample;
}

void Material_clearcoat::set_coating_attenuation(float3_p attenuation_color, float distance) {
    coating_.absorption_coef = attenuation_coefficient(attenuation_color, distance);
}

void Material_clearcoat::set_coating_ior(float ior) {
    coating_.ior = ior;
}

void Material_clearcoat::set_coating_roughness(float roughness) {
    float const r = ggx::clamp_roughness(roughness);

    coating_.alpha = r * r;
}

size_t Material_clearcoat::sample_size() {
    return sizeof(Sample_clearcoat);
}

Material_thinfilm::Material_thinfilm(Sampler_settings sampler_settings, bool two_sided)
    : Material_coating<Thinfilm_data>(sampler_settings, two_sided) {}

material::Sample const& Material_thinfilm::sample(float3_p wo, Renderstate const& rs,
                                                  Sampler& /*sampler*/, Worker&   worker) const {
    auto& sample = worker.sample<Sample_thinfilm>();

    auto& sampler = worker.sampler_2D(sampler_key(), rs.filter);

    set_sample(wo, rs, coating_.ior, sampler, worker, sample);

    set_coating_basis(wo, rs, sampler, worker, sample);

    sample.coating_.set(coating_.ior, ior_, coating_.alpha, coating_.thickness);

    return sample;
}

void Material_thinfilm::set_thinfilm(float ior, float roughness, float thickness) {
    coating_.ior = ior;

    float const r = ggx::clamp_roughness(roughness);

    coating_.alpha = r * r;

    coating_.thickness = thickness;
}

size_t Material_thinfilm::sample_size() {
    return sizeof(Sample_thinfilm);
}

}  // namespace scene::material::substitute
