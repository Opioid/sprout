#include "substitute_translucent_material.hpp"
#include "base/math/vector4.inl"

#include "scene/material/material.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "substitute_base_material.inl"
#include "substitute_base_sample.inl"
#include "substitute_translucent_sample.hpp"

namespace scene::material::substitute {

Material_translucent::Material_translucent(Sampler_settings sampler_settings)
    : Material_base(sampler_settings, true) {}

material::Sample const& Material_translucent::sample(float3_p           wo, Ray const& /*ray*/,
                                                     Renderstate const& rs, Sampler& /*sampler*/,
                                                     Worker&            worker) const {
    auto& sample = worker.sample<Sample_translucent>();

    auto const& sampler = worker.sampler_2D(sampler_key(), rs.filter);

    set_sample(wo, rs, rs.ior, sampler, worker, sample);

    float thickness;

    thickness = thickness_;
    sample.set_transluceny(sample.base_.albedo_, thickness, attenuation_distance_, transparency_);

    return sample;
}

void Material_translucent::set_volumetric(float thickness, float attenuation_distance) {
    thickness_            = thickness;
    attenuation_distance_ = attenuation_distance;

    transparency_ = std::exp(-thickness * (1.f / attenuation_distance));
}

size_t Material_translucent::sample_size() {
    return sizeof(Sample_translucent);
}

}  // namespace scene::material::substitute
