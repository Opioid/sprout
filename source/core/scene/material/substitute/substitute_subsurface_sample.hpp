#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_SAMPLE_HPP

#include "scene/material/disney/disney.hpp"
#include "scene/material/volumetric/volumetric_sample.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

class Sample_subsurface final : public Sample_base<disney::Isotropic_no_lambert> {
  public:
    bxdf::Result evaluate(float3 const& wi, bool include_back) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    bool do_evaluate_back(bool previously, bool same_side) const noexcept override final;

    void set_volumetric(float anisotropy, float ior, float ior_outside) noexcept;

  private:
    void refract(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept;

    float anisotropy_;

    IoR ior_;
};

class Sample_subsurface_volumetric final : public volumetric::Sample {
  public:
    bxdf::Result evaluate(float3 const& wi, bool include_back) const noexcept override final;

    bool do_evaluate_back(bool previously, bool same_side) const noexcept override final;

    void set(float anisotropy, float f0) noexcept;

  private:
    float f0_;
};

}  // namespace scene::material::substitute

#endif
