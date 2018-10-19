#pragma once

#include "base/math/distribution/distribution_2d.hpp"
#include "image/texture/texture.hpp"
#include "scene/material/material.hpp"

namespace scene::material::light {

class Emissionmap_animated : public Material {
  public:
    Emissionmap_animated(Sampler_settings const& sampler_settings, bool two_sided,
                         Texture_adapter const& emission_map, float emission_factor,
                         uint64_t animation_duration) noexcept;

    ~Emissionmap_animated() noexcept override;

    void tick(float absolute_time, float time_slice) noexcept override final;

    material::Sample const& sample(float3 const& wo, Renderstate const& rs, Sampler_filter filter,
                                   sampler::Sampler& sampler, Worker const& worker,
                                   uint32_t depth) const noexcept override;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Sampler_filter filter,
                             Worker const& worker) const noexcept override final;

    float3 average_radiance(float area) const noexcept override final;

    float ior() const noexcept override;

    bool has_emission_map() const noexcept override final;

    Sample_2D radiance_sample(float2 r2) const noexcept override final;

    float emission_pdf(float2 uv, Sampler_filter filter, Worker const& worker) const
        noexcept override final;

    float opacity(float2 uv, uint64_t time, Sampler_filter filter, Worker const& worker) const
        noexcept override final;

    void prepare_sampling(shape::Shape const& shape, uint32_t part, uint64_t time,
                          Transformation const& transformation, float area,
                          bool importance_sampling, thread::Pool& pool) noexcept override final;

    bool is_animated() const noexcept override final;

    size_t num_bytes() const noexcept override;

  protected:
    Texture_adapter emission_map_;

    math::Distribution_2D distribution_;

    float3 average_emission_;

    float emission_factor_;

    float total_weight_;

    uint64_t const frame_length_;

    int32_t element_;
};

}  // namespace scene::material::light
