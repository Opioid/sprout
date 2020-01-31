#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::volumetric {

class Sample : public material::Sample {
  public:
    float3 const& base_shading_normal() const noexcept override;

    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept override;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept override;

    void sample(Sampler& sampler, bxdf::Sample& result) const noexcept final;

    bool is_translucent() const noexcept final;

    bool evaluates_back(bool previously, bool same_side) const noexcept override;

    void set(float anisotropy);

  private:
    float phase(float3 const& wo, float3 const& wi) const noexcept;

    float4 sample(float3 const& wo, float2 r2) const noexcept;

    float anisotropy_;
};

}  // namespace scene::material::volumetric

#endif
