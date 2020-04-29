#ifndef SU_SCENE_MATERIAL_LIGHT_CONSTANT_HPP
#define SU_SCENE_MATERIAL_LIGHT_CONSTANT_HPP

#include "scene/light/emittance.hpp"
#include "scene/material/material.hpp"

namespace scene::material::light {

class Constant : public Material {
  public:
    Constant(Sampler_settings const& sampler_settings, bool two_sided);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             Worker const& worker) const final;

    float3 average_radiance(float area, Scene const& scene) const final;

    ::light::Emittance& emittance();

    static size_t sample_size();

  private:
    ::light::Emittance emittance_;
};

}  // namespace scene::material::light

#endif
