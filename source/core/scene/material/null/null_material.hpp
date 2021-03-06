#ifndef SU_CORE_SCENE_MATERIAL_NULL_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_NULL_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::null {

class alignas(16) Material : public material::Material {
  public:
    Material(Sampler_settings sampler_settings);

    material::Sample const& sample(float3_p wo, Renderstate const& rs, Sampler& sampler,
                                   Worker& worker) const final;
};

}  // namespace scene::material::null

#endif
