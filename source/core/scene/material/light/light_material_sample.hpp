#ifndef SU_SCENE_MATERIAL_LIGHT_SAMPLE_HPP
#define SU_SCENE_MATERIAL_LIGHT_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::light {

class alignas(64) Sample : public material::Sample {
  public:
    Sample();

    float3 const& base_shading_normal() const final;

    bxdf::Result evaluate_f(float3 const& wi) const final;

    bxdf::Result evaluate_b(float3 const& wi) const final;

    float3 radiance() const final;

    void sample(Sampler& sampler, bxdf::Sample& result) const final;

    void set(float3 const& radiance);

  private:
    float3 radiance_;
};

}  // namespace scene::material::light

#endif
