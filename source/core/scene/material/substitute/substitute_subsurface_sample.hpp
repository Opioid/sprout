#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_SAMPLE_HPP

#include "scene/material/disney/disney.hpp"
#include "scene/material/volumetric/volumetric_sample.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

class Sample_subsurface final : public Sample_base {
  public:
    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const final;

    void sample(Sampler& sampler, bxdf::Sample& result) const final;

    bool evaluates_back(bool previously, bool same_side) const final;

    void set_volumetric(float ior, float ior_outside);

    Base_closure<disney::Isotropic_no_lambert> base_;

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3 const& wi, bool include_back) const;

    void refract(Sampler& sampler, bxdf::Sample& result) const;

    IoR ior_;
};

class Sample_subsurface_volumetric final : public volumetric::Sample {
  public:
    bool evaluates_back(bool previously, bool same_side) const final;
};

}  // namespace scene::material::substitute

#endif
