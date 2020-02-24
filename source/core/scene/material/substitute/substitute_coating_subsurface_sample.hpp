#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SUBSURFACE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SUBSURFACE_SAMPLE_HPP

#include "scene/material/disney/disney.hpp"
#include "scene/material/volumetric/volumetric_sample.hpp"
#include "substitute_coating_sample.hpp"

namespace scene::material::substitute {

class Sample_coating_subsurface final
    : public Sample_coating<coating::Clearcoat_layer, disney::Isotropic_no_lambert> {
  public:
    bxdf::Result evaluate_f(float3 const& wi) const final;

    bxdf::Result evaluate_b(float3 const& wi) const final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const final;

    void set_volumetric(float anisotropy, float ior, float ior_outside);

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3 const& wi) const;

    void refract(sampler::Sampler& sampler, bxdf::Sample& result) const;

    float anisotropy_;

    IoR ior_;
};

class Sample_coating_subsurface_volumetric final : public volumetric::Sample {
  public:
    float3 const& base_shading_normal() const final;

    bxdf::Result evaluate_f(float3 const& wi) const final;

    bxdf::Result evaluate_b(float3 const& wi) const final;

  private:
    float3 attenuation(float3 const& wi) const;

  public:
    Layer layer_;

    coating::Clearcoat_layer coating_;
};

}  // namespace scene::material::substitute

#endif
