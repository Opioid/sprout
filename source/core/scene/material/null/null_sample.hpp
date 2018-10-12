#ifndef SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::null {

class Sample final : public material::Sample {
  public:
    Layer const& base_layer() const noexcept override final;

    bxdf::Result evaluate(float3 const& wi, bool include_back = true) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    bool ior_greater_one() const noexcept override final;

    bool do_evaluate_back(bool previously, bool same_side) const noexcept override final;

  private:
    Layer layer_;
};

}  // namespace scene::material::null

#endif
