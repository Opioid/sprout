#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_SAMPLE_HPP

#include "scene/material/disney/disney.hpp"
#include "substitute_base_sample.hpp"

namespace scene::material::substitute {

class Sample_subsurface final : public Sample_base<disney::Isotropic_no_lambert> {
  public:
    bxdf::Result evaluate(float3 const& wi) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    void set(float anisotropy, float ior, float ior_outside) noexcept;

  private:
    void refract(bool same_side, Layer const& layer, sampler::Sampler& sampler,
                 bxdf::Sample& result) const noexcept;

    void reflect_internally(Layer const& layer, sampler::Sampler& sampler,
                            bxdf::Sample& result) const noexcept;

    float anisotropy_;

    struct IOR {
        float ior_i_;
        float ior_o_;
        float eta_i_;
        float eta_t_;
    };

    IOR ior_;
};

}  // namespace scene::material::substitute

#endif
