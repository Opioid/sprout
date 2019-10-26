#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::debug {

class alignas(32) Sample : public material::Sample {
  public:
    float3 const& base_shading_normal() const noexcept override final;

    float3 base_tangent_to_world(float3 const& v) const noexcept override final;

    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept override final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    Layer layer_;
};

}  // namespace scene::material::debug
