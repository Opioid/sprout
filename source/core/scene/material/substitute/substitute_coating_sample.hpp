#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_HPP

#include "scene/material/coating/coating.hpp"
#include "scene/material/disney/disney.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

template <typename Coat, typename Diff>
class Sample_coating : public material::Sample {
  public:
    using Sample = bxdf::Sample;

    bxdf::Result evaluate(float3_p wi) const override;

    void sample(Sampler& sampler, RNG& rng, Sample& result) const override;

  protected:
    void coating_reflect(float f, float n_dot_h, Sample& result) const;

    void diffuse_sample(Sampler& sampler, RNG& rng, float f, Sample& result) const;

    void gloss_sample(Sampler& sampler, RNG& rng, float f, Sample& result) const;

    void pure_gloss_sample(Sampler& sampler, RNG& rng, float f, Sample& result) const;

  public:
    Base_closure<Diff> base_;

    Coat coating_;
};

using Sample_clearcoat = Sample_coating<coating::Clearcoat_layer, disney::Iso>;
using Sample_thinfilm  = Sample_coating<coating::Thinfilm_layer, disney::Iso>;

}  // namespace scene::material::substitute

#endif
