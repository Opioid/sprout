#ifndef SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class Sample_rough final : public material::Sample {
  public:
    bxdf::Result evaluate_f(float3 const& wi) const final;

    bxdf::Result evaluate_b(float3 const& wi) const final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const final;

    void set(float3 const& refraction_color, float ior, float ior_outside, float alpha,
             bool avoid_caustics);

    Layer layer_;

    float3 color_;

    float f0_;
    float alpha_;

    IoR ior_;

    bool avoid_caustics_;

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3 const& wi) const;
};

}  // namespace scene::material::glass

#endif
