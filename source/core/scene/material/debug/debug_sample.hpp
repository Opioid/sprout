#ifndef SU_CORE_SCENE_MATERIAL_DEBUG_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_DEBUG_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::debug {

class alignas(32) Sample : public material::Sample {
  public:
    bxdf::Result evaluate_f(float3 const& wi) const final;

    bxdf::Result evaluate_b(float3 const& wi) const final;

    void sample(Sampler& sampler, bxdf::Sample& result) const final;

    Layer layer_;
};

}  // namespace scene::material::debug

#endif
