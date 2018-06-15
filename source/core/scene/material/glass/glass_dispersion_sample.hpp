#ifndef SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_SAMPLE_HPP

#include "glass_sample.hpp"

namespace scene::material::glass {

class Sample_dispersion final : public Sample {
  public:
    virtual void sample(sampler::Sampler& sampler, bool avoid_caustics,
                        bxdf::Sample& result) const override final;

    void set(float ior, float abbe, float wavelength);

  private:
    float ior_;
    float abbe_;
    float wavelength_;
};

}  // namespace scene::material::glass

#endif
