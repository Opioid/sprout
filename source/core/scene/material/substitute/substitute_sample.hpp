#pragma once

#include "scene/material/disney/disney.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

class Sample : public Sample_base<disney::Isotropic> {
  public:
    virtual bxdf::Result evaluate(f_float3 wi) const override final;

    virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;
};

}  // namespace scene::material::substitute
