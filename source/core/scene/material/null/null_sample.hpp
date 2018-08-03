#ifndef SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::null {

class Sample final : public material::Sample {
  public:
    virtual Layer const& base_layer() const override final;

    virtual bxdf::Result evaluate(float3 const& wi) const override final;

    virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

    virtual bool ior_greater_one() const override final;

  private:
    material::Sample::Layer layer_;
};

}  // namespace scene::material::null

#endif
