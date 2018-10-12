#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_HPP

#include "scene/material/coating/coating.hpp"
#include "scene/material/disney/disney.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

template <typename Coating_layer>
class Sample_coating : public Sample_base<disney::Isotropic> {
  public:
    bxdf::Result evaluate(float3 const& wi, bool include_back) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

  private:
    void diffuse_sample_and_coating(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept;

    void gloss_sample_and_coating(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept;

    void pure_gloss_sample_and_coating(sampler::Sampler& sampler, bxdf::Sample& result) const
        noexcept;

  public:
    Coating_layer coating_;
};

using Sample_clearcoat = Sample_coating<coating::Clearcoat_layer>;
using Sample_thinfilm  = Sample_coating<coating::Thinfilm_layer>;

}  // namespace scene::material::substitute

#endif
