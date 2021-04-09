#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HOMOGENEOUS_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HOMOGENEOUS_HPP

#include "volumetric_material.hpp"

namespace scene::material::volumetric {

class Homogeneous : public Material {
  public:
    Homogeneous(Sampler_settings sampler_settings);

    void commit(Threads& threads, Scene const& scene) final;

    float3 evaluate_radiance(float3_p wi, float3_p uvw, float volume, Filter filter,
                             Worker& worker) const final;
};

}  // namespace scene::material::volumetric

#endif
