#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_TRANSLUCENT_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_TRANSLUCENT_SAMPLE_HPP

#include "scene/material/disney/disney.hpp"
#include "scene/material/material_sample.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

class Sample_translucent : public material::Sample {
  public:
    bxdf::Result evaluate(float3_p wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const final;

    void set_transluceny(float3_p color, float thickness, float attenuation_distance,
                         float transparency);

    Base_closure<disney::Iso> base_;

  private:
    float3 attenuation_;

    float thickness_;
    float transparency_;
};

}  // namespace scene::material::substitute

#endif
