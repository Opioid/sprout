#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_TRANSLUCENT_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_TRANSLUCENT_SAMPLE_HPP

#include "scene/material/disney/disney.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

class Sample_translucent : public Sample_base {
  public:
    bxdf::Result evaluate_f(float3 const& wi) const final;

    bxdf::Result evaluate_b(float3 const& wi) const final;

    void sample(Sampler& sampler, bxdf::Sample& result) const final;

    bool is_translucent() const final;

    void set_transluceny(float3 const& color, float transparency, float thickness, float attenuation_distance);

    Base_closure<disney::Isotropic> base_;

  private:
    template <bool Forward>
    bxdf::Result evaluate(float3 const& wi) const;

    float3 attenuation_;

    float  transparency_;
    float  thickness_;
};

}  // namespace scene::material::substitute

#endif
