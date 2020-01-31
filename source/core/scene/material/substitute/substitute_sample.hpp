#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SAMPLE_HPP

#include "scene/material/disney/disney.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

class alignas(64) Sample : public Sample_base {
  public:
    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept final;

    void sample(Sampler& sampler, bxdf::Sample& result) const noexcept final;

    float3 radiance() const noexcept final;

    Base_closure<disney::Isotropic> base_;

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3 const& wi) const noexcept;
};

}  // namespace scene::material::substitute

#endif
