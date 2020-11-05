#include "light_emissionmap_animated.hpp"
#include "image/texture/texture.inl"
#include "image/texture/texture_adapter.inl"
#include "light_material_sample.hpp"
#include "scene/material/material.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::light {

Emissionmap_animated::Emissionmap_animated(Sampler_settings const& sampler_settings, bool two_sided)
    : Emissionmap(sampler_settings, two_sided) {
    properties_.set(Property::Animated);
}

Emissionmap_animated::~Emissionmap_animated() = default;

void Emissionmap_animated::simulate(uint64_t start, uint64_t /*end*/, uint64_t /*frame_length*/,
                                    Threads& /*threads*/, Scene const& scene) {
    uint64_t const num_elements = uint64_t(emission_map_.texture(scene).num_elements());

    int32_t const element = int32_t((start / (animation_duration_ / num_elements)) % num_elements);

    if (element != element_) {
        element_          = element;
        average_emission_ = float3(-1.f);
    }
}

material::Sample const& Emissionmap_animated::sample(float3 const&      wo, Ray const& /*ray*/,
                                                     Renderstate const& rs, Filter filter,
                                                     Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample>();

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    float3 const radiance  = emission_map_.sample_3(worker, sampler, rs.uv, element_);
    float3 const fradiance = emission_factor_ * radiance;

    sample.set_common(rs, wo, fradiance, fradiance, 0.f);

    return sample;
}

float3 Emissionmap_animated::evaluate_radiance(float3 const& /*wi*/, float3 const& uvw,
                                               float /*extent*/, Filter            filter,
                                               Worker const& worker) const {
    auto& sampler = worker.sampler_2D(sampler_key(), filter);
    return emission_factor_ * emission_map_.sample_3(worker, sampler, uvw.xy(), element_);
}

void Emissionmap_animated::prepare_sampling(Shape const& shape, uint32_t /*part*/, uint64_t time,
                                            Transformation const& /*trafo*/, float /*area*/,
                                            bool importance_sampling, Threads& threads,
                                            Scene const& scene) {
    uint64_t const num_elements = uint64_t(emission_map_.texture(scene).num_elements());

    int32_t const element = int32_t((time / (animation_duration_ / num_elements)) % num_elements);

    if (element == element_) {
        return;
    }

    element_ = element;

    prepare_sampling_internal(shape, element, importance_sampling, threads, scene);
}

void Emissionmap_animated::set_emission_map(Texture_adapter const& emission_map,
                                            uint64_t               animation_duration) {
    emission_map_ = emission_map;

    animation_duration_ = animation_duration;
}

}  // namespace scene::material::light
