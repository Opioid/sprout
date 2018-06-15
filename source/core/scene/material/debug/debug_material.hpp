#pragma once

#include "scene/material/material.hpp"

namespace scene::material::debug {

class Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings);

    virtual const material::Sample& sample(f_float3 wo, Renderstate const& rs,
                                           Sampler_filter filter, sampler::Sampler& sampler,
                                           Worker const& worker,
                                           uint32_t      depth) const override final;

    virtual float ior() const override final;

    virtual size_t num_bytes() const override final;

  private:
};

}  // namespace scene::material::debug
