#ifndef SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_SAMPLE_HPP

#include "glass_sample.hpp"

namespace scene::material::glass {

class Sample_dispersion final : public Sample {
  public:
    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const final;

    void set_dispersion(float abbe, float wavelength);

  private:
    float abbe_;
    float wavelength_;
};

}  // namespace scene::material::glass

#endif
