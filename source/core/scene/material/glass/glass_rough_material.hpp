#ifndef SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::glass {

class Glass_rough : public Material {
  public:
    Glass_rough(Sampler_settings sampler_settings);

    void commit(Threads& threads, Scene const& scene) final;

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    void set_normal_map(Turbotexture const& normal_map);
    void set_roughness_map(Turbotexture const& roughness_map);

    void set_refraction_color(float3_p color);
    void set_roughness(float roughness);

    static size_t sample_size();

  protected:
    Turbotexture normal_map_;
    Turbotexture roughness_map_;

    float3 refraction_color_;

    float alpha_;
};

}  // namespace scene::material::glass

#endif
