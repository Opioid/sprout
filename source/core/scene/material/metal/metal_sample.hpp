#pragma once

#include "base/math/vector2.hpp"
#include "scene/material/material_sample.hpp"

namespace scene::material::metal {

class Sample_isotropic : public material::Sample {
  public:
    virtual Layer const& base_layer() const override final;

    virtual bxdf::Result evaluate(f_float3 wi) const override final;

    virtual void sample(sampler::Sampler& sampler,
                        bxdf::Sample& result) const override final;

    struct Layer : material::Sample::Layer {
        void   set(float3 const& ior, float3 const& absorption, float roughness);
        float3 ior_;
        float3 absorption_;
        float  alpha_;
        float  alpha2_;
    };

    Layer layer_;
    bool avoid_caustics_;
};

class Sample_anisotropic : public material::Sample {
  public:
    virtual Layer const& base_layer() const override final;

    virtual bxdf::Result evaluate(f_float3 wi) const override final;

    virtual void sample(sampler::Sampler& sampler,
                        bxdf::Sample& result) const override final;

    struct Layer : material::Sample::Layer {
        void set(float3 const& ior, float3 const& absorption, float2 roughness);

        float3 ior_;
        float3 absorption_;
        float2 a_;
        float2 alpha2_;
        float  axy_;
    };

    Layer layer_;
    bool avoid_caustics_;
};

}  // namespace scene::material::metal
