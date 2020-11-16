#pragma once

#include "base/math/vector2.hpp"
#include "scene/material/material_sample.hpp"

namespace scene::material::metal {

class Sample_isotropic : public material::Sample {
  public:
    bxdf::Result evaluate_f(float3_p wi) const final;

    bxdf::Result evaluate_b(float3_p wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const final;

    void set(float3_p ior, float3_p absorption);

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3_p wi) const;

    float3 ior_;
    float3 absorption_;
};

class Sample_anisotropic : public material::Sample {
  public:
    bxdf::Result evaluate_f(float3_p wi) const final;

    bxdf::Result evaluate_b(float3_p wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const final;

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3_p wi) const;

  public:
    void set(float3_p ior, float3_p absorption, float2 alpha);

    float3 ior_;
    float3 absorption_;
    float2 alpha_;
};

}  // namespace scene::material::metal
