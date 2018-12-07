#pragma once

#include "substitute_base_material.hpp"

namespace scene::material::substitute {

class Material_translucent : public Material_base {
  public:
    Material_translucent(Sampler_settings const& sampler_settings) noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, sampler::Sampler& sampler,
                                   Worker const& worker) const noexcept override final;

    size_t num_bytes() const noexcept override final;

    void set_thickness(float thickness) noexcept;
    void set_attenuation_distance(float attenuation_distance) noexcept;

    static size_t sample_size() noexcept;

  private:
    float thickness_;
    float attenuation_distance_;
};

}  // namespace scene::material::substitute
