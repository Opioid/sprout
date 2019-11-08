#pragma once

#include "scene/light/emittance.hpp"
#include "scene/material/material.hpp"

namespace scene::material::light {

class Constant : public Material {
  public:
    Constant(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker const& worker) const
        noexcept override final;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             Worker const& worker) const noexcept override final;

    float3 average_radiance(float area, Scene const& scene) const noexcept override final;

    float ior() const noexcept override final;

    bool has_emission_map() const noexcept override final;

    size_t num_bytes() const noexcept override final;

    ::light::Emittance& emittance() noexcept;

    static size_t sample_size() noexcept;

  private:
    ::light::Emittance emittance_;
};

}  // namespace scene::material::light
