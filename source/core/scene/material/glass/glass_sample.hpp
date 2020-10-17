#ifndef SU_CORE_SCENE_MATERIAL_GLASS_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class alignas(64) Sample : public material::Sample {
  public:
    Sample();

    bxdf::Result evaluate_f(float3 const& wi) const final;

    bxdf::Result evaluate_b(float3 const& wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const override;

    void set(float3 const& refraction_color, float ior, float ior_outside);

    void sample(float ior, float p, bxdf::Sample& result) const;

    Layer layer_;

    float3 color_;

    float ior_;
    float ior_outside_;
};

}  // namespace scene::material::glass

#endif
