#include "substitute_translucent_material.hpp"
#include "base/math/vector4.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "substitute_base_material.inl"
#include "substitute_base_sample.inl"
#include "substitute_translucent_sample.hpp"

namespace scene::material::substitute {

Material_translucent::Material_translucent(Sampler_settings const& sampler_settings)
    : Material_base(sampler_settings, true) {}

material::Sample const& Material_translucent::sample(float3 const&      wo, Ray const& /*ray*/,
                                                     Renderstate const& rs, Filter filter,
                                                     Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample_translucent>();

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, rs.ior, sampler, worker, sample);

    float thickness;

    thickness = thickness_;
    sample.set_transluceny(sample.base_.diffuse_color_, thickness, attenuation_distance_,
                           transparency_);

    return sample;
}

void Material_translucent::set_attenuation(float thickness, float attenuation_distance) {
    thickness_            = thickness;
    attenuation_distance_ = attenuation_distance;

    transparency_ = std::exp(-thickness * (1.f / attenuation_distance));
}

size_t Material_translucent::sample_size() {
    return sizeof(Sample_translucent);
}

}  // namespace scene::material::substitute
