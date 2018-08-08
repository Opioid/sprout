#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::matte {

class Sample : public material::Sample {
  public:
    Layer const& base_layer() const noexcept override final;

    bxdf::Result evaluate(float3 const& wi) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    struct Layer : public material::Sample::Layer {
        void set(float3 const& color) noexcept;

        float3 diffuse_color_;
        float  roughness_;
        float  alpha_;
        float  alpha2_;
    };

    Layer layer_;
};

}  // namespace scene::material::matte
