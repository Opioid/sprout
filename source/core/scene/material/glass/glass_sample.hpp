#ifndef SU_CORE_SCENE_MATERIAL_GLASS_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class Sample : public material::Sample {
  public:
    Layer const& base_layer() const noexcept override final;

    bxdf::Result evaluate(float3 const& wi, bool include_back) const noexcept override final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept override;

    void set(float3 const& refraction_color, float ior, float ior_outside) noexcept;

    void sample(float ior, float p, bxdf::Sample& result) const noexcept;

    Layer layer_;

    float3 color_;

    float ior_;
    float ior_outside_;

    static void reflect(float3 const& wo, float3 const& n, float n_dot_wo,
                        bxdf::Sample& result) noexcept;

    static void refract(float3 const& wo, float3 const& n, float3 const& color, float n_dot_wo,
                        float n_dot_t, float eta, bxdf::Sample& result) noexcept;
};

}  // namespace scene::material::glass

#endif
