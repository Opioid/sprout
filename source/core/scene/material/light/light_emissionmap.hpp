#pragma once

#include "base/math/distribution/distribution_2d.hpp"
#include "scene/material/material.hpp"

namespace scene::material::light {

class Emissionmap : public Material {
  public:
    Emissionmap(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    ~Emissionmap() noexcept override;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                           Filter filter, sampler::Sampler& sampler,
                                           Worker const& worker, uint32_t sample_level) const
        noexcept override;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                                     Worker const& worker) const noexcept override final;

    float3 average_radiance(float area) const noexcept override final;

    float ior() const noexcept override;

    bool has_emission_map() const noexcept override final;

    Sample_2D radiance_sample(float2 r2) const noexcept override final;

    float emission_pdf(float2 uv, Filter filter, Worker const& worker) const
        noexcept override final;

    void prepare_sampling(shape::Shape const& shape, uint32_t part, uint64_t time,
                                  Transformation const& transformation, float area,
                                  bool          importance_sampling,
                                  thread::Pool& pool) noexcept override final;

    size_t num_bytes() const noexcept override;

    void set_emission_map(Texture_adapter const& emission_map) noexcept;
    void set_emission_factor(float emission_factor) noexcept;

  protected:
    Texture_adapter emission_map_;

    math::Distribution_2D distribution_;

    float3 average_emission_;

    float emission_factor_;

    float total_weight_;
};

}  // namespace scene::material::light
