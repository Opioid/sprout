#pragma once

#include "scene/material/light/light_material_sample.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene::material::sky {

class Material_overcast : public Material {
  public:
    Material_overcast(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker const& worker) const
        noexcept final;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             Worker const& worker) const noexcept final;

    float3 average_radiance(float area, Scene const& scene) const noexcept final;

    float ior() const noexcept final;

    size_t num_bytes() const noexcept final;

    void set_emission(float3 const& radiance) noexcept;

  private:
    float3 overcast(float3 const& wi) const noexcept;

    float3 color_;
};

}  // namespace scene::material::sky
