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
                                   uint32_t sample_level) const noexcept override final;

    float ior() const noexcept override final;

    CM control_medium() const noexcept override final;

    float van_de_hulst_scattering_scale(uint32_t depth) const noexcept override final;

    void set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
                         float distance) noexcept;

    void set_anisotropy(float anisotropy) noexcept;

    static size_t sample_size() noexcept;

    static void set_van_de_hulst_range(uint32_t low, uint32_t high);

  protected:
    float van_de_hulst_anisotropy(uint32_t depth) const noexcept;

    CC cc_;
    CM cm_;

    float anisotropy_;

  private:
    static uint32_t VdH_low;
    static uint32_t VdH_high;
    static float    VdH_inv_range;
};

}  // namespace scene::material::volumetric

#endif
