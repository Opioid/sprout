#ifndef SU_CORE_SCENE_MATERIAL_GLASS_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::glass {

class Glass : public Material {
  public:
    Glass(Sampler_settings const& sampler_settings);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const override;

    void set_normal_map(Texture_adapter const& normal_map);

    void set_refraction_color(float3 const& color);

    static size_t sample_size();

  protected:
    Texture_adapter normal_map_;

    float3 refraction_color_;
};

}  // namespace scene::material::glass

#endif
