#ifndef SU_CORE_SCENE_MATERIAL_GLASS_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::glass {

class alignas(64) Glass : public Material {
  public:
    Glass(Sampler_settings const& sampler_settings);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler,
                                   Worker const& worker) const override;

    float3 absorption_coefficient(float2 uv, Filter filter, Worker const& worker) const final;

    float ior() const final;

    size_t num_bytes() const override;

    void set_normal_map(Texture_adapter const& normal_map);

    void set_refraction_color(float3 const& color);
    void set_attenuation(float3 const& absorption_color, float distance);
    void set_ior(float ior);

    bool is_caustic() const final;

    static size_t sample_size();

  protected:
    Texture_adapter normal_map_;

    float3 refraction_color_;
    float3 absorption_coefficient_;
    float3 absorption_color_;
    float  attenuation_distance_;
    float  ior_;
};

}  // namespace scene::material::glass

#endif
