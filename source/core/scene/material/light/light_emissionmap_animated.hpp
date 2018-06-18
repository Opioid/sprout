#pragma once

#include "base/math/distribution/distribution_2d.hpp"
#include "image/texture/texture.hpp"
#include "scene/material/material.hpp"

namespace scene::material::light {

class Emissionmap_animated : public Material {
  public:
    Emissionmap_animated(Sampler_settings const& sampler_settings, bool two_sided,
                         Texture_adapter const& emission_map, float emission_factor,
                         float animation_duration);

    ~Emissionmap_animated();

    virtual void tick(float absolute_time, float time_slice) override final;

    virtual material::Sample const& sample(f_float3 wo, Renderstate const& rs,
                                           Sampler_filter filter, sampler::Sampler& sampler,
                                           Worker const& worker, uint32_t depth) const override;

    virtual float3 sample_radiance(f_float3 wi, float2 uv, float area, float time,
                                   Sampler_filter filter,
                                   Worker const&  worker) const override final;

    virtual float3 average_radiance(float area) const override final;

    virtual float ior() const override;

    virtual bool has_emission_map() const override final;

    virtual Sample_2D radiance_sample(float2 r2) const override final;

    virtual float emission_pdf(float2 uv, Sampler_filter filter,
                               Worker const& worker) const override final;

    virtual float opacity(float2 uv, float time, Sampler_filter filter,
                          Worker const& worker) const override final;

    virtual void prepare_sampling(shape::Shape const& shape, uint32_t part,
                                  Transformation const& transformation, float area,
                                  bool importance_sampling, thread::Pool& pool) override final;

    virtual bool is_animated() const override final;

    virtual size_t num_bytes() const override;

  protected:
    Texture_adapter emission_map_;

    math::Distribution_2D distribution_;

    float3 average_emission_;

    float emission_factor_;

    float total_weight_;

    float const frame_length_;

    int32_t element_;
};

}  // namespace scene::material::light
