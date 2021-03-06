#ifndef SU_CORE_SCENE_MATERIAL_GLASS_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::glass {

class Glass : public Material {
  public:
    Glass(Sampler_settings sampler_settings);

    material::Sample const& sample(float3_p wo, Renderstate const& rs, Sampler& sampler,
                                   Worker& worker) const override;

    void set_normal_map(Texture const& normal_map);

    void set_abbe(float abbe);

    static size_t sample_size();

  protected:
    Texture normal_map_;

    float abbe_;
};

}  // namespace scene::material::glass

#endif
