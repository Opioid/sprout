#pragma once

#include "scene/material/material.hpp"

namespace scene::material::debug {

class Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings) noexcept;

    material::Sample const& sample(float3 const& wo, Renderstate const& rs, Filter filter,
                                   sampler::Sampler& sampler, Worker const& worker,
                                   uint32_t depth) const noexcept override final;

    float ior() const noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
};

}  // namespace scene::material::debug
