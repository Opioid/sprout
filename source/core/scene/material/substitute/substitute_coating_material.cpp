#include "substitute_coating_material.inl"
#include "base/math/vector4.inl"
#include "scene/material/coating/coating.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "substitute_coating_sample.inl"

namespace scene::material::substitute {

Material_clearcoat::Material_clearcoat(Sampler_settings const& sampler_settings, bool two_sided)
    : Material_coating<coating::Clearcoat>(sampler_settings, two_sided) {}

material::Sample const& Material_clearcoat::sample(float3 const& wo, Renderstate const& rs,
                                                   Sampler_filter filter,
                                                   sampler::Sampler& /*sampler*/,
                                                   Worker const& worker, uint32_t depth) const {
    auto& sample = worker.sample<Sample_clearcoat>(depth);

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, sampler, sample);

    set_coating_basis(wo, rs, sampler, sample);

    sample.coating_.set(coating_.f0_, coating_.alpha_, coating_.alpha2_);

    return sample;
}

void Material_clearcoat::set_clearcoat(float ior, float roughness) {
    coating_.f0_      = fresnel::schlick_f0(1.f, ior);
    roughness         = ggx::clamp_roughness(roughness);
    float const alpha = roughness * roughness;
    coating_.alpha_   = alpha;
    coating_.alpha2_  = alpha * alpha;
}

size_t Material_clearcoat::sample_size() {
    return sizeof(Sample_clearcoat);
}

Material_thinfilm::Material_thinfilm(Sampler_settings const& sampler_settings, bool two_sided)
    : Material_coating<coating::Thinfilm>(sampler_settings, two_sided) {}

material::Sample const& Material_thinfilm::sample(float3 const& wo, Renderstate const& rs,
                                                  Sampler_filter filter,
                                                  sampler::Sampler& /*sampler*/,
                                                  Worker const& worker, uint32_t depth) const {
    auto& sample = worker.sample<Sample_thinfilm>(depth);

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, sampler, sample);

    set_coating_basis(wo, rs, sampler, sample);

    sample.coating_.set(coating_.ior_, coating_.alpha_, coating_.alpha2_, coating_.thickness_);

    return sample;
}

void Material_thinfilm::set_thinfilm(float ior, float roughness, float thickness) {
    coating_.ior_       = ior;
    roughness           = ggx::clamp_roughness(roughness);
    float const alpha   = roughness * roughness;
    coating_.alpha_     = alpha;
    coating_.alpha2_    = alpha * alpha;
    coating_.thickness_ = thickness;
}

size_t Material_thinfilm::sample_size() {
    return sizeof(Sample_thinfilm);
}

}  // namespace scene::material::substitute
