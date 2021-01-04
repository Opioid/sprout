#ifndef SU_CORE_SCENE_MATERIAL_DEBUG_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_DEBUG_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::debug {

class Sample : public material::Sample {
  public:
    bxdf::Result evaluate(float3_p wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const final;
};

}  // namespace scene::material::debug

#endif
