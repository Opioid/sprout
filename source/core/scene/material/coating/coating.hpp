#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::coating {

struct Result {
    float3 reflection;
    float3 attenuation;
    float  pdf;
};

struct Coating_base {
    void set_color_and_weight(float3 const& color, float weight) noexcept;

    float3 color_;
    float  weight_;
};

struct Clearcoat_data : public Coating_base {
    float ior_;
    float alpha_;
    float alpha2_;
};

class Clearcoat : public Coating_base {
  public:
    void set(float f0, float alpha, float alpha2) noexcept;

  protected:
    template <typename Layer>
    Result evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                    Layer const& layer, bool avoid_caustics) const noexcept;

    template <typename Layer>
    void sample(float3 const& wo, Layer const& layer, sampler::Sampler& sampler,
                float3& attenuation, bxdf::Sample& result) const noexcept;

  public:
    float f0_;
    float alpha_;
    float alpha2_;
};

class Thinfilm : public Coating_base {
  public:
    void set(float ior, float ior_internal, float alpha, float alpha2, float thickness) noexcept;

  protected:
    template <typename Layer>
    Result evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                    Layer const& layer, bool avoid_caustics) const noexcept;

    template <typename Layer>
    void sample(float3 const& wo, Layer const& layer, sampler::Sampler& sampler,
                float3& attenuation, bxdf::Sample& result) const noexcept;

  public:
    float ior_;
    float ior_internal_;
    float alpha_;
    float alpha2_;
    float thickness_;
};

template <typename Coating>
class Coating_layer : public Sample::Layer, public Coating {
  public:
    Result evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h, bool avoid_caustics) const
        noexcept;

    void sample(float3 const& wo, sampler::Sampler& sampler, float3& attenuation,
                bxdf::Sample& result) const noexcept;
};

using Clearcoat_layer = Coating_layer<Clearcoat>;
using Thinfilm_layer  = Coating_layer<Thinfilm>;

}  // namespace scene::material::coating
