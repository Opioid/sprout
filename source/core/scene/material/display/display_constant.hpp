#pragma once

#include "scene/material/material.hpp"

namespace scene::material::display {

class Constant : public Material {
  public:
    Constant(Sampler_settings const& sampler_settings, bool two_sided);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             Worker const& worker) const final;

    float3 average_radiance(float area, Scene const& scene) const final;

    void set_emission(float3 const& radiance);

    void set_roughness(float roughness);

    static size_t sample_size();

  private:
    Texture_adapter emission_map_;

    float3 emission_;

    float alpha_;
};

}  // namespace scene::material::display
