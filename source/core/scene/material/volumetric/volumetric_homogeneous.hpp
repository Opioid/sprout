#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HOMOGENEOUS_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HOMOGENEOUS_HPP

#include "volumetric_material.hpp"

namespace scene::material::volumetric {

class Homogeneous : public Material {
  public:
    Homogeneous(Sampler_settings const& sampler_settings);

    void commit(thread::Pool& threads, Scene const& scene) final;

    float3 evaluate_radiance(float3 const& wi, float3 const& uvw, float volume, Filter filter,
                             Worker const& worker) const final;
};

}  // namespace scene::material::volumetric

#endif
