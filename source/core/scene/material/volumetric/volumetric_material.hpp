#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::volumetric {

class Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings) noexcept;

    ~Material() noexcept override;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, sampler::Sampler& sampler, Worker const& worker,
                                   uint32_t depth) const noexcept override final;

    float ior() const noexcept override final;

    CM control_medium() const noexcept override final;

    float van_de_hulst_scale(float towards_zero) const noexcept override final;

    void set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
                         float distance) noexcept;

    void set_anisotropy(float anisotropy) noexcept;

    static size_t sample_size() noexcept;

  protected:
    CC cc_;
    CM cm_;

    float anisotropy_;
};

}  // namespace scene::material::volumetric

#endif
