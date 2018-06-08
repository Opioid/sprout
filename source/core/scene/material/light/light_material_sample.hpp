#ifndef SU_SCENE_MATERIAL_LIGHT_SAMPLE_HPP
#define SU_SCENE_MATERIAL_LIGHT_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::light {

class Sample : public material::Sample {
  public:
    virtual Layer const& base_layer() const override final;

    virtual bxdf::Result evaluate(f_float3 wi) const override final;

    virtual float3 radiance() const override final;

    virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

    virtual bool is_pure_emissive() const override final;

    struct Layer : public material::Sample::Layer {
        void set(float3 const& radiance);

        float3 radiance_;
    };

    Layer layer_;
};

}  // namespace scene::material::light

#endif
