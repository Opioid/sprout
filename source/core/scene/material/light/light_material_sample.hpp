#ifndef SU_SCENE_MATERIAL_LIGHT_SAMPLE_HPP
#define SU_SCENE_MATERIAL_LIGHT_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::light {

class Sample : public material::Sample {
  public:
    Layer const& base_layer() const noexcept override final;

    bxdf::Result evaluate(float3 const& wi) const noexcept override final;

    float3 radiance() const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    bool is_pure_emissive() const noexcept override final;

    void set(float3 const& radiance) noexcept;

    Layer layer_;

    float3 radiance_;
};

}  // namespace scene::material::light

#endif
