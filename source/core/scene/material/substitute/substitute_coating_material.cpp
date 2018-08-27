#include "substitute_coating_material.inl"
#include "base/math/vector4.inl"
#include "scene/material/coating/coating.inl"
#include "scene/material/material_attenuation.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "substitute_coating_sample.inl"

namespace scene::material::substitute {

Material_clearcoat::Material_clearcoat(Sampler_settings const& sampler_settings,
                                       bool                    two_sided) noexcept
    : Material_coating<coating::Clearcoat_data>(sampler_settings, two_sided) {}

material::Sample const& Material_clearcoat::sample(float3 const& wo, Renderstate const& rs,
                                                   Sampler_filter filter,
                                                   sampler::Sampler& /*sampler*/,
                                                   Worker const& worker, uint32_t depth) const
    noexcept {
    auto& sample = worker.sample<Sample_clearcoat>(depth);

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, coating_.ior_, sampler, sample);

    set_coating_basis(wo, rs, sampler, sample);

    sample.coating_.set(coating_.absorption_coefficient_, 1.f,
                        fresnel::schlick_f0(coating_.ior_, rs.ior), coating_.alpha_);

    return sample;
}

void Material_clearcoat::set_coating_attenuation(float3 const& absorption_color,
                                                 float         distance) noexcept {
    coating_.absorption_coefficient_ = scene::material::extinction_coefficient(absorption_color,
                                                                               distance);
}

void Material_clearcoat::set_clearcoat(float ior, float roughness) noexcept {
    coating_.ior_ = ior;

    float const r     = ggx::clamp_roughness(roughness);
    float const alpha = r * r;

    coating_.alpha_ = alpha;
}

size_t Material_clearcoat::sample_size() noexcept {
    return sizeof(Sample_clearcoat);
}

Material_thinfilm::Material_thinfilm(Sampler_settings const& sampler_settings,
                                     bool                    two_sided) noexcept
    : Material_coating<coating::Thinfilm>(sampler_settings, two_sided) {}

material::Sample const& Material_thinfilm::sample(float3 const& wo, Renderstate const& rs,
                                                  Sampler_filter filter,
                                                  sampler::Sampler& /*sampler*/,
                                                  Worker const& worker, uint32_t depth) const
    noexcept {
    auto& sample = worker.sample<Sample_thinfilm>(depth);

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, coating_.ior_, sampler, sample);

    set_coating_basis(wo, rs, sampler, sample);

    sample.coating_.set(coating_.ior_, ior_, coating_.alpha_, coating_.thickness_);

    return sample;
}

void Material_thinfilm::set_thinfilm(float ior, float roughness, float thickness) noexcept {
    coating_.ior_ = ior;

    float const r     = ggx::clamp_roughness(roughness);
    float const alpha = r * r;

    coating_.alpha_     = alpha;
    coating_.thickness_ = thickness;
}

size_t Material_thinfilm::sample_size() noexcept {
    return sizeof(Sample_thinfilm);
}

}  // namespace scene::material::substitute
