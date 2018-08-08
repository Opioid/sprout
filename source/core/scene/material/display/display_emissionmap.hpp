#pragma once

#include "scene/material/light/light_emissionmap.hpp"

namespace scene::material::display {

class Emissionmap : public light::Emissionmap {
  public:
    Emissionmap(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    virtual material::Sample const& sample(float3 const& wo, Renderstate const& rs,
                                           Sampler_filter filter, sampler::Sampler& sampler,
                                           Worker const& worker, uint32_t depth) const
        noexcept override final;

    virtual float ior() const noexcept override final;

    virtual size_t num_bytes() const noexcept override final;

    void set_roughness(float roughness) noexcept;
    void set_ior(float ior) noexcept;

  private:
    float roughness_;

    float ior_;

    float f0_;
};

}  // namespace scene::material::display
