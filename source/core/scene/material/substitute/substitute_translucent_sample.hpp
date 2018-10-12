#pragma once

#include "scene/material/disney/disney.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

class Sample_translucent : public Sample_base<disney::Isotropic> {
  public:
    bxdf::Result evaluate(float3 const& wi, bool include_back) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    bool is_translucent() const noexcept override final;

    void set_transluceny(float3 const& diffuse_color, float thickness,
                         float attenuation_distance) noexcept;

  private:
    float3 attenuation_;
    float  thickness_;
};

}  // namespace scene::material::substitute
