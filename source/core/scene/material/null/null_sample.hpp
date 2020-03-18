#ifndef SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::null {

class Sample final : public material::Sample {
  public:
    Sample();

    float3 const& base_shading_normal() const final;

    bxdf::Result evaluate_f(float3 const& wi) const final;

    bxdf::Result evaluate_b(float3 const& wi) const final;

    void sample(Sampler& sampler, bxdf::Sample& result) const final;

    float factor_ = 1.f;
};

}  // namespace scene::material::null

#endif
