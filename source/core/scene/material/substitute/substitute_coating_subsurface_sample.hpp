#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SUBSURFACE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SUBSURFACE_SAMPLE_HPP

#include "scene/material/disney/disney.hpp"
#include "scene/material/volumetric/volumetric_sample.hpp"
#include "substitute_coating_sample.hpp"

namespace scene::material::substitute {

class Sample_coating_subsurface final
    : public Sample_coating<coating::Clearcoat_layer, disney::Isotropic_no_lambert> {
  public:
    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept override final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    bool do_evaluate_back(bool previously, bool same_side) const noexcept override final;

    void set_volumetric(float anisotropy, float ior, float ior_outside) noexcept;

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3 const& wi, bool include_back) const noexcept;

    void refract(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept;

    float anisotropy_;

    IoR ior_;
};

class Sample_coating_subsurface_volumetric final : public volumetric::Sample {
  public:
    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept override final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept override final;

    bool do_evaluate_back(bool previously, bool same_side) const noexcept override final;

    void set(float anisotropy, float f0) noexcept;

  private:
    float f0_;

  public:
    coating::Clearcoat_layer coating_;
};

}  // namespace scene::material::substitute

#endif
