#ifndef SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class Sample_rough final : public material::Sample {
  public:
    Layer const& base_layer() const noexcept override final;

    bxdf::Result evaluate(float3 const& wi) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    void set(float3 const& refraction_color, float ior, float ior_outside, float alpha) noexcept;

    struct Layer : public material::Sample::Layer {
        float alpha_;
    };

    Layer layer_;

    float3 color_;

    float f0_;

    IoR ior_;

    void reflect(Layer const& layer, sampler::Sampler& sampler, bxdf::Sample& result) const
        noexcept;

    void reflect_internally(Layer const& layer, sampler::Sampler& sampler,
                            bxdf::Sample& result) const noexcept;

    void refract(bool same_side, Layer const& layer, sampler::Sampler& sampler,
                 bxdf::Sample& result) const noexcept;
};

}  // namespace scene::material::glass

#endif
