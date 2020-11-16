#ifndef SU_CORE_SCENE_MATERIAL_GLASS_THIN_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_THIN_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class Sample_thin : public material::Sample {
  public:
    Sample_thin();

    bxdf::Result evaluate_f(float3_p wi) const final;

    bxdf::Result evaluate_b(float3_p wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const final;

    void set(float3_p absorption_coef, float ior, float ior_outside, float thickness);

    float3 absorption_coef_;

    float ior_;
    float ior_outside_;
    float thickness_;
};

}  // namespace scene::material::glass

#endif
