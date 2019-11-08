#ifndef SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class Sample_rough final : public material::Sample {
  public:
    float3 const& base_shading_normal() const noexcept override final;

    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept override final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    void set(float3 const& refraction_color, float ior, float ior_outside, float alpha,
             bool avoid_caustics) noexcept;

    Layer layer_;

    float3 color_;

    float f0_;
    float alpha_;

    IoR ior_;

    bool avoid_caustics_;

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3 const& wi) const noexcept;
};

}  // namespace scene::material::glass

#endif
