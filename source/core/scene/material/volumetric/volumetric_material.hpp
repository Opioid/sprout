#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::volumetric {

class Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings);

    ~Material() override;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    float3 average_radiance(float volume, Scene const& scene) const override;

    void set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
                         float distance);

    static size_t sample_size();

  protected:
};

}  // namespace scene::material::volumetric

#endif
