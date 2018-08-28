#ifndef SU_CORE_SCENE_MATERIAL_COATING_COATING_HPP
#define SU_CORE_SCENE_MATERIAL_COATING_COATING_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::coating {

struct Result {
    float3 reflection;
    float3 attenuation;
    float  pdf;
};

class Clearcoat {
  public:
    void set(float3 const& absorption_coefficient, float thickness, float f0, float alpha) noexcept;

  protected:
    template <typename Layer>
    Result evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                    Layer const& layer, bool avoid_caustics) const noexcept;

    template <typename Layer>
    void sample(float3 const& wo, Layer const& layer, sampler::Sampler& sampler,
                float3& attenuation, bxdf::Sample& result) const noexcept;

  public:
    float3 absorption_coefficient_;

    float thickness_;
    float f0_;
    float alpha_;
};

class Thinfilm {
  public:
    void set(float ior, float ior_internal, float alpha, float thickness) noexcept;

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
    float thickness_;
};

template <typename Coating>
class Coating_layer : public Sample::Layer, public Coating {
  public:
    Result evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                    bool avoid_caustics) const noexcept;

    void sample(float3 const& wo, sampler::Sampler& sampler, float3& attenuation,
                bxdf::Sample& result) const noexcept;
};

using Clearcoat_layer = Coating_layer<Clearcoat>;
using Thinfilm_layer  = Coating_layer<Thinfilm>;

}  // namespace scene::material::coating

#endif
