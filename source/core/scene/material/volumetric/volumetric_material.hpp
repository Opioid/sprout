#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::volumetric {

class Material : public material::Material {
  public:
    Material(Sampler_settings sampler_settings);

    ~Material() override;

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Sampler& sampler, Worker& worker) const final;

    float3 average_radiance(float volume) const override;

    static size_t sample_size();

  protected:
};

}  // namespace scene::material::volumetric

#endif
