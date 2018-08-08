#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::display {

class Sample : public material::Sample {
  public:
    Layer const& base_layer() const noexcept override final;

    bxdf::Result evaluate(float3 const& wi) const noexcept override final;

    float3 radiance() const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    struct Layer : material::Sample::Layer {
        void set(float3 const& radiance, float f0, float roughness) noexcept;

        float3 emission_;
        float3 f0_;
        float  alpha_;
        float  alpha2_;
    };

    Layer layer_;
};

}  // namespace scene::material::display
