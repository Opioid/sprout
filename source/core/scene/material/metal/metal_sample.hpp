#pragma once

#include "base/math/vector2.hpp"
#include "scene/material/material_sample.hpp"

namespace scene::material::metal {

class Sample_isotropic : public material::Sample {
  public:
    Layer const& base_layer() const noexcept override final;

    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept override final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    void set(float3 const& ior, float3 const& absorption, float alpha,
             bool avoid_caustics) noexcept;

    Layer layer_;

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3 const& wi) const noexcept;

    float3 ior_;
    float3 absorption_;

    float alpha_;

    bool avoid_caustics_;
};

class Sample_anisotropic : public material::Sample {
  public:
    Layer const& base_layer() const noexcept override final;

    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept override final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3 const& wi) const noexcept;

  public:
    struct PLayer : material::Layer {
        void set(float3 const& ior, float3 const& absorption, float2 roughness) noexcept;

        float3 ior_;
        float3 absorption_;
        float2 a_;
        float2 alpha2_;
        float  axy_;
    };

    PLayer layer_;

    bool avoid_caustics_;
};

}  // namespace scene::material::metal
