#ifndef SU_CORE_SCENE_MATERIAL_GLASS_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class Sample : public material::Sample {
  public:
    Sample();

    bxdf::Result evaluate(float3_p wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const override;

    void set(float ior, float ior_outside);

    void sample(float ior, float p, bxdf::Sample& result) const;

    float ior_;
    float ior_outside_;
};

}  // namespace scene::material::glass

#endif
