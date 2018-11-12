#pragma once

#include "scene/material/light/light_emissionmap.hpp"

namespace scene::material::display {

class Emissionmap : public light::Emissionmap {
  public:
    Emissionmap(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                           Filter filter, sampler::Sampler& sampler,
                                           Worker const& worker, uint32_t sample_level) const
        noexcept override final;

    float ior() const noexcept override final;

    size_t num_bytes() const noexcept override final;

    void set_roughness(float roughness) noexcept;
    void set_ior(float ior) noexcept;

  private:
    float alpha_;

    float ior_;

    float f0_;
};

}  // namespace scene::material::display
