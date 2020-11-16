#ifndef SU_CORE_SCENE_MATERIAL_DISPLAY_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_DISPLAY_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::display {

class Sample : public material::Sample {
  public:
    bxdf::Result evaluate_f(float3_p wi) const final;

    bxdf::Result evaluate_b(float3_p wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const final;

    void set(float f0);

    float f0_;
};

}  // namespace scene::material::display

#endif
