#include "substitute_translucent_material.hpp"
#include "base/math/vector4.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "substitute_base_material.inl"
#include "substitute_base_sample.inl"
#include "substitute_translucent_sample.hpp"

namespace scene::material::substitute {

Material_translucent::Material_translucent(Sampler_settings const& sampler_settings) noexcept
    : Material_base(sampler_settings, true) {}

material::Sample const& Material_translucent::sample(float3 const& wo, Renderstate const& rs,
                                                     Sampler_filter filter,
                                                     sampler::Sampler& /*sampler*/,
                                                     Worker const& worker, uint32_t depth) const
    noexcept {
    auto& sample = worker.sample<Sample_translucent>(depth);

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, rs.ior, sampler, sample);

    float thickness;

    thickness = thickness_;
    sample.set_transluceny(sample.diffuse_color_, thickness, attenuation_distance_);

    return sample;
}

size_t Material_translucent::num_bytes() const noexcept {
    return sizeof(*this);
}

void Material_translucent::set_thickness(float thickness) noexcept {
    thickness_ = thickness;
}

void Material_translucent::set_attenuation_distance(float attenuation_distance) noexcept {
    attenuation_distance_ = attenuation_distance;
}

size_t Material_translucent::sample_size() noexcept {
    return sizeof(Sample_translucent);
}

}  // namespace scene::material::substitute
