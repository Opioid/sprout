#pragma once

#include "scene/material/material.hpp"

namespace scene::material::display {

class Constant : public Material {
  public:
    Constant(Sampler_settings const& sampler_settings, bool two_sided);

    virtual material::Sample const& sample(float3 const& wo, Renderstate const& rs,
                                           Sampler_filter filter, sampler::Sampler& sampler,
                                           Worker const& worker,
                                           uint32_t      depth) const override final;

    virtual float3 evaluate_radiance(float3 const& wi, float2 uv, float area, float time,
                                     Sampler_filter filter,
                                     Worker const&  worker) const override final;

    virtual float3 average_radiance(float area) const override final;

    virtual float ior() const override final;

    virtual size_t num_bytes() const override final;

    void set_emission(float3 const& radiance);
    void set_roughness(float roughness);
    void set_ior(float ior);

    static size_t sample_size();

  private:
    Texture_adapter emission_map_;

    float3 emission_;

    float roughness_;

    float ior_;

    float f0_;
};

}  // namespace scene::material::display
