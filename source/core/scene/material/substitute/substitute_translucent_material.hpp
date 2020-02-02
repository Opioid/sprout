#pragma once

#include "substitute_base_material.hpp"

namespace scene::material::substitute {

class Material_translucent : public Material_base {
  public:
    Material_translucent(Sampler_settings const& sampler_settings);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler,
                                   Worker const& worker) const final;

    size_t num_bytes() const final;

    void set_thickness(float thickness);
    void set_attenuation_distance(float attenuation_distance);

    static size_t sample_size();

  private:
    float thickness_;
    float attenuation_distance_;
};

}  // namespace scene::material::substitute
