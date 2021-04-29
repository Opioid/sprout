#ifndef SU_CORE_SCENE_MATERIAL_GLASS_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::glass {

class Glass : public Material {
  public:
    Glass(Sampler_settings sampler_settings);

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const override;

    void set_normal_map(Turbotexture const& normal_map);

    void set_refraction_color(float3_p color);

    static size_t sample_size();

  protected:
    Turbotexture normal_map_;

    float3 refraction_color_;
};

}  // namespace scene::material::glass

#endif
