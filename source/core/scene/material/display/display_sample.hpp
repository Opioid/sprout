#ifndef SU_CORE_SCENE_MATERIAL_DISPLAY_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_DISPLAY_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::display {

class Sample : public material::Sample {
  public:
    bxdf::Result evaluate_f(float3 const& wi) const final;

    bxdf::Result evaluate_b(float3 const& wi) const final;

    void sample(Sampler& sampler, rnd::Generator& rng, bxdf::Sample& result) const final;

    void set(float3 const& radiance, float f0, float alpha);

    Layer layer_;

    float f0_;
    float alpha_;
};

}  // namespace scene::material::display

#endif
