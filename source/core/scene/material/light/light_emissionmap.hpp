#pragma once

#include "base/math/distribution/distribution_2d.hpp"
#include "scene/material/material.hpp"

namespace scene::material::light {

class Emissionmap : public Material {
  public:
    Emissionmap(Sampler_settings const& sampler_settings, bool two_sided);
    ~Emissionmap();

    virtual const material::Sample& sample(f_float3 wo, Renderstate const& rs,
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

    virtual void prepare_sampling(shape::Shape const& shape, uint32_t part,
                                  Transformation const& transformation, float area,
                                  bool importance_sampling, thread::Pool& pool) override final;

    virtual size_t num_bytes() const override;

    void set_emission_map(Texture_adapter const& emission_map);
    void set_emission_factor(float emission_factor);

  protected:
    Texture_adapter emission_map_;

    math::Distribution_2D distribution_;

    float3 average_emission_;

    float emission_factor_;

    float total_weight_;
};

}  // namespace scene::material::light
