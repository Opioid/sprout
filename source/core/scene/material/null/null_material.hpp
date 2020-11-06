#ifndef SU_CORE_SCENE_MATERIAL_NULL_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_NULL_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::null {

class alignas(16) Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;
};

}  // namespace scene::material::null

#endif
