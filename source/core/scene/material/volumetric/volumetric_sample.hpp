#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::volumetric {

class Sample final : public material::Sample {
  public:
    virtual Layer const& base_layer() const override final;

    virtual bxdf::Result evaluate(float3 const& wi) const override final;

    virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

    virtual bool is_translucent() const override final;

    void set(float anisotropy);

  public:
    struct Layer : public material::Sample::Layer {
        float phase(float3 const& wo, float3 const& wi) const;

        float sample(float3 const& wo, float2 r2, float3& wi) const;

        float anisotropy;
    };

    Layer layer_;

  private:
    static float phase_hg(float cos_theta, float g);

    static float phase_schlick(float cos_theta, float k);
};

}  // namespace scene::material::volumetric

#endif
