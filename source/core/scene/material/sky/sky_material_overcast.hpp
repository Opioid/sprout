#pragma once

#include "scene/material/light/light_material_sample.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene::material::sky {

class Material_overcast : public Material {
  public:
    Material_overcast(Sampler_settings const& sampler_settings, bool two_sided);

    virtual material::Sample const& sample(f_float3 wo, Renderstate const& rs,
                                           Sampler_filter filter, sampler::Sampler& sampler,
                                           Worker const& worker,
                                           uint32_t      depth) const override final;

    virtual float3 evaluate_radiance(f_float3 wi, float2 uv, float area, float time,
                                     Sampler_filter filter,
                                     Worker const&  worker) const override final;

    virtual float3 average_radiance(float area) const override final;

    virtual float ior() const override final;

    virtual size_t num_bytes() const override final;

    void set_emission(float3 const& radiance);

  private:
    float3 overcast(f_float3 wi) const;

    float3 color_;
};

}  // namespace scene::material::sky
