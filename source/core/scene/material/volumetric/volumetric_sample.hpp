#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::volumetric {

class Sample : public material::Sample {
  public:
    Sample();

    bxdf::Result evaluate_f(float3_p wi) const override;

    bxdf::Result evaluate_b(float3_p wi) const override;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const final;

    void set(float anisotropy);

  private:
    float phase(float3_p wo, float3_p wi) const;

    float4 sample(float3_p wo, float2 r2) const;

    float anisotropy_;
};

}  // namespace scene::material::volumetric

#endif
