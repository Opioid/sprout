#ifndef SU_CORE_SCENE_MATERIAL_GLASS_THIN_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_THIN_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class Sample_thin : public material::Sample {
  public:
    float3 const& base_shading_normal() const noexcept final;

    bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept final;

    bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept final;

    void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept final;

    bool is_translucent() const noexcept final;

    void set(float3 const& refraction_color, float3 const& absorption_coefficient, float ior,
             float ior_outside, float thickness) noexcept;

    Layer layer_;

    float3 color_;
    float3 absorption_coefficient_;

    float ior_;
    float ior_outside_;
    float thickness_;

    static void reflect(float3 const& wo, float3 const& n, float n_dot_wo,
                        bxdf::Sample& result) noexcept;

    static void refract(float3 const& wo, float3 const& color, bxdf::Sample& result) noexcept;
};

}  // namespace scene::material::glass

#endif
