#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::cloth {

class Sample : public material::Sample {
  public:
    float3 const& base_shading_normal() const final;

    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const final;

    void set(float3 const& color);

    Layer layer_;

    float3 diffuse_color_;
};

}  // namespace scene::material::cloth
