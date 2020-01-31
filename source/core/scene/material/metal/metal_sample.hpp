#pragma once

#include "base/math/vector2.hpp"
#include "scene/material/material_sample.hpp"

namespace scene::material::metal {

class alignas(64) Sample_isotropic : public material::Sample {
  public:
    float3 const& base_shading_normal() const noexcept final;

    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept final;

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

class alignas(64) Sample_anisotropic : public material::Sample {
  public:
    float3 const& base_shading_normal() const noexcept final;

    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept final;

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
