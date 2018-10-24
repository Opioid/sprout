#pragma once

#include "scene/material/material.hpp"

namespace scene::material::display {

class Constant : public Material {
  public:
    Constant(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    material::Sample const& sample(float3 const& wo, Renderstate const& rs, Filter filter,
                                   sampler::Sampler& sampler, Worker const& worker,
                                   uint32_t depth) const noexcept override final;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             Worker const& worker) const noexcept override final;

    float3 average_radiance(float area) const noexcept override final;

    float ior() const noexcept override final;

    size_t num_bytes() const noexcept override final;

    void set_emission(float3 const& radiance) noexcept;
    void set_roughness(float roughness) noexcept;
    void set_ior(float ior) noexcept;

    static size_t sample_size() noexcept;

  private:
    Texture_adapter emission_map_;

    float3 emission_;

    float ior_;
    float alpha_;
    float f0_;
};

}  // namespace scene::material::display
