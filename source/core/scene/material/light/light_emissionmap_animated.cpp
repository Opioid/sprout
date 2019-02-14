#include "light_emissionmap_animated.hpp"
#include "image/texture/texture_adapter.inl"
#include "light_material_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::light {

Emissionmap_animated::Emissionmap_animated(Sampler_settings const& sampler_settings,
                                           bool                    two_sided) noexcept
    : Emissionmap(sampler_settings, two_sided), element_(-1) {}

Emissionmap_animated::~Emissionmap_animated() noexcept {}

void Emissionmap_animated::simulate(uint64_t start, uint64_t /*end*/, uint64_t /*frame_length*/,
                                    thread::Pool& /*pool*/) noexcept {
    int32_t const element = static_cast<int32_t>(start / frame_length_) %
                            emission_map_.texture().num_elements();

    if (element != element_) {
        element_          = element;
        average_emission_ = float3(-1.f);
    }
}

material::Sample const& Emissionmap_animated::sample(float3 const&      wo, Ray const& /*ray*/,
                                                     Renderstate const& rs, Filter filter,
                                                     Sampler& /*sampler*/,
                                                     Worker const& worker) const noexcept {
    auto& sample = worker.sample<Sample>(rs.sample_level);

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    float3 const radiance = emission_map_.sample_3(sampler, rs.uv, element_);

    sample.set(emission_factor_ * radiance);

    return sample;
}

float3 Emissionmap_animated::evaluate_radiance(float3 const& /*wi*/, float2 uv, float /*area*/,
                                               Filter filter, Worker const& worker) const noexcept {
    auto& sampler = worker.sampler_2D(sampler_key(), filter);
    return emission_factor_ * emission_map_.sample_3(sampler, uv, element_);
}

float Emissionmap_animated::opacity(float2 uv, uint64_t /*time*/, Filter filter,
                                    Worker const& worker) const noexcept {
    if (mask_.is_valid()) {
        auto& sampler = worker.sampler_2D(sampler_key(), filter);
        return mask_.sample_1(sampler, uv, element_);
    } else {
        return 1.f;
    }
}

void Emissionmap_animated::prepare_sampling(Shape const& shape, uint32_t /*part*/, uint64_t time,
                                            Transformation const& /*transformation*/,
                                            float /*area*/, bool importance_sampling,
                                            thread::Pool& pool) noexcept {
    int32_t const element = static_cast<int32_t>(
        (time / frame_length_) % static_cast<uint64_t>(emission_map_.texture().num_elements()));

    if (element == element_) {
        return;
    }

    element_ = element;

    prepare_sampling_internal(shape, element, importance_sampling, pool);
}

bool Emissionmap_animated::is_animated() const noexcept {
    return true;
}

void Emissionmap_animated::set_emission_map(Texture_adapter const& emission_map,
                                            uint64_t               animation_duration) noexcept {
    emission_map_ = emission_map;

    frame_length_ = animation_duration /
                    static_cast<uint64_t>(emission_map.texture().num_elements());
}

size_t Emissionmap_animated::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::material::light
