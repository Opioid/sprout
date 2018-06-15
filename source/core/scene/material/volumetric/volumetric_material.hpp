#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::volumetric {

class Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings);
    virtual ~Material() override;

    virtual const material::Sample& sample(f_float3 wo, Renderstate const& rs,
                                           Sampler_filter filter, sampler::Sampler& sampler,
                                           Worker const& worker,
                                           uint32_t      depth) const override final;

    virtual float ior() const override final;

    void set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
                         float distance);

    void set_anisotropy(float anisotropy);

    static size_t sample_size();

  protected:
    float3 absorption_coefficient_;
    float3 scattering_coefficient_;
    float  anisotropy_;
};

}  // namespace scene::material::volumetric

#endif
