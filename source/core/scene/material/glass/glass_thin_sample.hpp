#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class Sample_thin : public material::Sample {
  public:
    Layer const& base_layer() const noexcept override final;

    bxdf::Result evaluate(float3 const& wi) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override final;

    bool is_translucent() const noexcept override final;

    struct Layer : public material::Layer {
        void set(float3 const& refraction_color, float3 const& absorption_coefficient, float ior,
                 float ior_outside, float thickess) noexcept;

        float3 color_;
        float3 absorption_coefficient_;
        float  ior_;
        float  ior_outside_;
        float  thickness_;
    };

    Layer layer_;

    class BSDF {
      public:
        static float reflect(const Sample_thin& sample, Layer const& layer,
                             sampler::Sampler& sampler, bxdf::Sample& result) noexcept;

        static float refract(const Sample_thin& sample, Layer const& layer,
                             sampler::Sampler& sampler, bxdf::Sample& result) noexcept;
    };
};

}  // namespace scene::material::glass
