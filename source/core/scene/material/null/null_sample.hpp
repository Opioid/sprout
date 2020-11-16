#ifndef SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::null {

class Sample final : public material::Sample {
  public:
    Sample();

    bxdf::Result evaluate_f(float3_p wi) const final;

    bxdf::Result evaluate_b(float3_p wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const final;

    float factor_ = 1.f;
};

}  // namespace scene::material::null

#endif
