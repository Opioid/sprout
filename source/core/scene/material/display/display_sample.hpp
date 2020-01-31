#ifndef SU_CORE_SCENE_MATERIAL_DISPLAY_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_DISPLAY_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::display {

class Sample : public material::Sample {
  public:
    float3 const& base_shading_normal() const noexcept final;

    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept final;

    float3 radiance() const noexcept final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept final;

    void set(float3 const& radiance, float f0, float alpha) noexcept;

    Layer layer_;

    float3 emission_;

    float f0_;
    float alpha_;
};

}  // namespace scene::material::display

#endif
