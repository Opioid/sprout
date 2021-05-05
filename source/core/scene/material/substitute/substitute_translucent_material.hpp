#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_TRANSLUCENT_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_TRANSLUCENT_MATERIAL_HPP

#include "substitute_base_material.hpp"

namespace scene::material::substitute {

class Material_translucent : public Material_base {
  public:
    Material_translucent(Sampler_settings sampler_settings);

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Sampler& sampler, Worker& worker) const final;

    void set_volumetric(float thickness, float attenuation_distance);

    static size_t sample_size();

  private:
    float thickness_;
    float transparency_;
};

}  // namespace scene::material::substitute

#endif
