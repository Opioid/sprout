#include "light_emissionmap_animated.hpp"
#include "image/texture/texture.inl"

#include "light_material_sample.hpp"
#include "scene/material/material.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::light {

Emissionmap_animated::Emissionmap_animated(Sampler_settings sampler_settings, bool two_sided)
    : Emissionmap(sampler_settings, two_sided) {
    properties_.set(Property::Animated);
    properties_.set(Property::Pure_emissive);
}

Emissionmap_animated::~Emissionmap_animated() = default;

void Emissionmap_animated::simulate(uint64_t start, uint64_t /*end*/, uint64_t /*frame_length*/,
                                    Threads& /*threads*/, Scene const& scene) {
    uint64_t const num_elements = uint64_t(emission_map_.description(scene).num_elements());

    int32_t const element = int32_t((start / (animation_duration_ / num_elements)) % num_elements);

    if (element != element_) {
        element_          = element;
        average_emission_ = float3(-1.f);
    }
}

material::Sample const& Emissionmap_animated::sample(float3_p           wo, Ray const& /*ray*/,
                                                     Renderstate const& rs, Filter filter,
                                                     Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample>();

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    float3 const radiance  = sampler.sample_3(emission_map_, rs.uv, element_, worker.scene());
    float3 const fradiance = emission_factor_ * radiance;

    sample.set_common(rs, wo, fradiance, fradiance, 0.f);

    return sample;
}

float3 Emissionmap_animated::evaluate_radiance(float3_p /*wi*/, float3_p uvw, float /*extent*/,
                                               Filter filter, Worker const& worker) const {
    auto& sampler = worker.sampler_2D(sampler_key(), filter);
    return emission_factor_ * sampler.sample_3(emission_map_, uvw.xy(), element_, worker.scene());
}

void Emissionmap_animated::set_emission_map(Texture const& emission_map,
                                            uint64_t       animation_duration) {
    Emissionmap::set_emission_map(emission_map);

    animation_duration_ = animation_duration;
}

}  // namespace scene::material::light
