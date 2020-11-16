#ifndef SU_SCENE_MATERIAL_LIGHT_SAMPLE_HPP
#define SU_SCENE_MATERIAL_LIGHT_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::light {

class Sample : public material::Sample {
  public:
    Sample();

    bxdf::Result evaluate_f(float3_p wi) const final;

    bxdf::Result evaluate_b(float3_p wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const final;
};

}  // namespace scene::material::light

#endif
