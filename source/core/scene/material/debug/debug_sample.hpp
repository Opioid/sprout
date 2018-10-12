#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::debug {

class Sample : public material::Sample {
  public:
    Layer const& base_layer() const noexcept override final;

    bxdf::Result evaluate(float3 const& wi, bool include_back) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    Layer layer_;
};

}  // namespace scene::material::debug
