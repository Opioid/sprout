#pragma once

#include "scene/material/disney/disney.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

class Sample : public Sample_base<disney::Isotropic> {
  public:
    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept override final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept override final;

    void sample(Sampler& sampler, bxdf::Sample& result) const noexcept override final;

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3 const& wi) const noexcept;
};

}  // namespace scene::material::substitute
