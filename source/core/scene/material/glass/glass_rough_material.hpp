#ifndef SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::glass {

class Glass_rough : public Material {
  public:
    Glass_rough(Sampler_settings const& sampler_settings);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    void set_normal_map(Texture const& normal_map);
    void set_roughness_map(Texture const& roughness_map);

    void set_refraction_color(float3 const& color);
    void set_attenuation(float3 const& absorption_color, float distance);
    void set_roughness(float roughness);

    static size_t sample_size();

  protected:
    Texture normal_map_;
    Texture roughness_map_;

    float3 refraction_color_;

    float alpha_;
};

}  // namespace scene::material::glass

#endif
