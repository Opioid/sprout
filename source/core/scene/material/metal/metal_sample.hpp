#ifndef SU_CORE_SCENE_MATERIAL_METAL_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_METAL_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::metal {

class Sample : public material::Sample {
  public:
    bxdf::Result evaluate(float3_p wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const final;

    void set(float3_p ior, float3_p absorption);

  private:
    float3 ior_;
    float3 absorption_;
};

}  // namespace scene::material::metal

#endif
