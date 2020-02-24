#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::volumetric {

class Sample : public material::Sample {
  public:
    float3 const& base_shading_normal() const override;

    bxdf::Result evaluate_f(float3 const& wi) const override;

    bxdf::Result evaluate_b(float3 const& wi) const override;

    void sample(Sampler& sampler, bxdf::Sample& result) const final;

    bool is_translucent() const final;

    void set(float anisotropy);

  private:
    float phase(float3 const& wo, float3 const& wi) const;

    float4 sample(float3 const& wo, float2 r2) const;

    float anisotropy_;
};

}  // namespace scene::material::volumetric

#endif
