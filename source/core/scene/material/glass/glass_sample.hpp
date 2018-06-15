#ifndef SU_CORE_SCENE_MATERIAL_GLASS_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class Sample : public material::Sample {
  public:
    virtual Layer const& base_layer() const override final;

    virtual bxdf::Result evaluate(f_float3 wi) const override final;

    virtual void sample(sampler::Sampler& sampler,
                        bxdf::Sample& result) const override;

    virtual bool is_transmissive() const override final;

    struct Layer : public material::Sample::Layer {
        void set(float3 const& refraction_color, float3 const& absorption_color,
                 float attenuation_distance, float ior, float ior_outside);

        float3        color_;
        float3        absorption_coefficient_;
        mutable float ior_;
        float         ior_outside_;
    };

    Layer layer_;

    class BSDF {
      public:
        static float reflect(const Sample& sample, Layer const& layer, sampler::Sampler& sampler,
                             bxdf::Sample& result);

        static float refract(const Sample& sample, Layer const& layer, sampler::Sampler& sampler,
                             bxdf::Sample& result);
    };
};

}  // namespace scene::material::glass

#endif
