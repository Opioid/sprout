#pragma once

#include "scene/material/material.hpp"

namespace scene::material::mix {

class Material : public material::Material {
  public:
    Material(Sampler_settings sampler_settings, bool two_sided);

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    void set_materials(uint32_t a, uint32_t const b);

  private:
    uint32_t material_a_;
    uint32_t material_b_;
};

}  // namespace scene::material::mix
