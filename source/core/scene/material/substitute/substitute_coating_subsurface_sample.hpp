#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SUBSURFACE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SUBSURFACE_SAMPLE_HPP

#include "scene/material/disney/disney.hpp"
#include "scene/material/volumetric/volumetric_sample.hpp"
#include "substitute_coating_sample.hpp"

namespace scene::material::substitute {

class Sample_coating_subsurface final
    : public Sample_coating<coating::Clearcoat_layer, disney::Isotropic_no_lambert> {
  public:
    using Diffuse = disney::Isotropic_no_lambert;

    bxdf::Result evaluate_f(float3_p wi) const final;

    bxdf::Result evaluate_b(float3_p wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const final;

    void set_volumetric(float ior, float ior_outside);

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3_p wi) const;

    IoR ior_;
};

}  // namespace scene::material::substitute

#endif
