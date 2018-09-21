#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::volumetric {

class Sample : public material::Sample {
  public:
    Layer const& base_layer() const noexcept override final;

    bxdf::Result evaluate(float3 const& wi) const noexcept override;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    bool is_translucent() const noexcept override final;

    void set(float anisotropy);

  private:
    float phase(float3 const& wo, float3 const& wi) const noexcept;

    float sample(float3 const& wo, float2 r2, float3& wi) const noexcept;

    Layer layer_;

    float anisotropy_;
};

}  // namespace scene::material::volumetric

#endif
