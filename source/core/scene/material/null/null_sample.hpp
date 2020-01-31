#ifndef SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::null {

class Sample final : public material::Sample {
  public:
    float3 const& base_shading_normal() const noexcept final;

    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept final;

    void sample(Sampler& sampler, bxdf::Sample& result) const noexcept final;

    bool ior_greater_one() const noexcept final;

    bool evaluates_back(bool previously, bool same_side) const noexcept final;
};

}  // namespace scene::material::null

#endif
