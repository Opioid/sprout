#ifndef SU_CORE_SCENE_MATERIAL_DEBUG_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_DEBUG_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::debug {

class alignas(16) Material : public material::Material {
  public:
    Material(Sampler_settings sampler_settings);

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Sampler& sampler, Worker& worker) const final;
};

}  // namespace scene::material::debug

#endif
