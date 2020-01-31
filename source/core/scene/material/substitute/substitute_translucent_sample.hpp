#pragma once

#include "scene/material/disney/disney.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

class Sample_translucent : public Sample_base {
  public:
    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept final;

    bool is_translucent() const noexcept final;

    void set_transluceny(float3 const& diffuse_color, float thickness,
                         float attenuation_distance) noexcept;

    Base_closure<disney::Isotropic> base_;

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3 const& wi, bool include_back) const noexcept;

    float3 attenuation_;
    float  thickness_;
};

}  // namespace scene::material::substitute
