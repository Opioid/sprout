#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::cloth {

class Sample : public material::Sample {
  public:
    virtual Layer const& base_layer() const override final;

    virtual bxdf::Result evaluate(f_float3 wi) const override final;

    virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

    struct Layer : public material::Sample::Layer {
        void set(float3 const& color);

        float3 diffuse_color;
    };

    Layer layer_;
};

}  // namespace scene::material::cloth
