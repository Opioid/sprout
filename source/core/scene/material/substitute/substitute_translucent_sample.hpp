#pragma once

#include "scene/material/disney/disney.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

class Sample_translucent : public Sample_base<disney::Isotropic> {
  public:
    virtual bxdf::Result evaluate(f_float3 wi, bool avoid_caustics) const override final;

    virtual void sample(sampler::Sampler& sampler, bool avoid_caustics,
                        bxdf::Sample& result) const override final;

    virtual bool is_translucent() const override final;

    void set(float3 const& diffuse_color, float thickness, float attenuation_distance);

  private:
    float3 attenuation_;
    float  thickness_;
};

}  // namespace scene::material::substitute
