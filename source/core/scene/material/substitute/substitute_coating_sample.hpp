#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_HPP

#include "scene/material/coating/coating.hpp"
#include "scene/material/disney/disney.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

template <typename Coating_layer, typename Diffuse>
class Sample_coating : public Sample_base {
  public:
    float3 radiance() const noexcept final;

    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept override;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept override;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override;

  protected:
    template <bool Forward>
    bxdf::Result evaluate(float3 const& wi) const noexcept;

    void coating_sample_and_base(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept;

    void diffuse_sample_and_coating(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept;

    void gloss_sample_and_coating(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept;

    void pure_gloss_sample_and_coating(sampler::Sampler& sampler, bxdf::Sample& result) const
        noexcept;

  public:
    Base_closure<Diffuse> base_;

    Coating_layer coating_;
};

using Sample_clearcoat = Sample_coating<coating::Clearcoat_layer, disney::Isotropic>;
using Sample_thinfilm  = Sample_coating<coating::Thinfilm_layer, disney::Isotropic>;

}  // namespace scene::material::substitute

#endif
