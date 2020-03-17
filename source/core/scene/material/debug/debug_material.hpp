#pragma once

#include "scene/material/material.hpp"

namespace scene::material::debug {

class alignas(32) Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    float ior() const final;

    size_t num_bytes() const final;

  private:
};

}  // namespace scene::material::debug
