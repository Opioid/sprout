#ifndef SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::null {

class Sample final : public material::Sample {
  public:
    float3 const& base_shading_normal() const noexcept override final;

    float3 base_tangent_to_world(float3 const& v) const noexcept override final;

    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept override final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept override final;

    void sample(Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    bool ior_greater_one() const noexcept override final;

    bool do_evaluate_back(bool previously, bool same_side) const noexcept override final;

  private:
    Layer layer_;
};

}  // namespace scene::material::null

#endif
