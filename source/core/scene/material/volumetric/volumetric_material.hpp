#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::volumetric {

class Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings);

    ~Material() override;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler,
                                   Worker const& worker) const final;

    float ior() const final;

    CM control_medium() const final;

    float similarity_relation_scale(uint32_t depth) const final;

    bool is_scattering_volume() const final;

    float3 average_radiance(float area, Scene const& scene) const final;

    void set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
                         float distance);

    void set_emission(float3 const& emission);

    void set_anisotropy(float anisotropy);

    static size_t sample_size();

    static void set_similarity_relation_range(uint32_t low, uint32_t high);

  protected:
    float van_de_hulst_anisotropy(uint32_t depth) const;

    CC cc_;
    CM cm_;

    float3 emission_;

    float distance_;

    float anisotropy_;

    bool is_scattering_;

  private:
    static uint32_t SR_low;
    static uint32_t SR_high;
    static float    SR_inv_range;
};

}  // namespace scene::material::volumetric

#endif
