#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_HPP

#include "scene/material/coating/coating.hpp"
#include "scene/material/disney/disney.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

template <typename Coating_layer, typename Diffuse>
class Sample_coating : public Sample_base {
  public:
    bxdf::Result evaluate_f(float3 const& wi) const override;

    bxdf::Result evaluate_b(float3 const& wi) const override;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const override;

  protected:
    template <bool Forward>
    bxdf::Result evaluate(float3 const& wi) const;

    void coating_sample_and_base(Sampler& sampler, RNG& rng, bxdf::Sample& result) const;

    void diffuse_sample_and_coating(Sampler& sampler, RNG& rng, bxdf::Sample& result) const;

    void gloss_sample_and_coating(Sampler& sampler, RNG& rng, bxdf::Sample& result) const;

    void pure_gloss_sample_and_coating(Sampler& sampler, RNG& rng, bxdf::Sample& result) const;

  public:
    Base_closure<Diffuse> base_;

    Coating_layer coating_;
};

using Sample_clearcoat = Sample_coating<coating::Clearcoat_layer, disney::Isotropic>;
using Sample_thinfilm  = Sample_coating<coating::Thinfilm_layer, disney::Isotropic>;

}  // namespace scene::material::substitute

#endif
