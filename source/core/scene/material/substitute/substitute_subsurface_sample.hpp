#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_SAMPLE_HPP

#include "scene/material/disney/disney.hpp"
#include "scene/material/volumetric/volumetric_sample.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

class Sample_subsurface final : public material::Sample {
  public:
    using Diffuse = disney::Isotropic_no_lambert;

    bxdf::Result evaluate_f(float3_p wi) const final;

    bxdf::Result evaluate_b(float3_p wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const final;

    void set_volumetric(float ior, float ior_outside);

    Base_closure<Diffuse> base_;

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3_p wi) const;

    IoR ior_;
};

}  // namespace scene::material::substitute

#endif
