#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::substitute {

template <typename Diffuse>
class Sample_base : public material::Sample {
  public:
    Layer const& base_layer() const noexcept override final;

    float3 radiance() const noexcept override final;

    void set(float3 const& color, float3 const& radiance, float f0, float alpha, float metallic,
             bool avoid_caustics) noexcept;

    bxdf::Result base_evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                               bool avoid_caustics) const noexcept;

    bxdf::Result pure_gloss_evaluate(float3 const& wi, float3 const& wo, float3 const& h,
                                     float wo_dot_h, bool avoid_caustics) const noexcept;

    void diffuse_sample(float3 const& wo, Sampler& sampler, bool avoid_caustics,
                        bxdf::Sample& result) const noexcept;

    void gloss_sample(float3 const& wo, Sampler& sampler, bxdf::Sample& result) const noexcept;

    void pure_gloss_sample(float3 const& wo, Sampler& sampler, bxdf::Sample& result) const noexcept;

    float base_diffuse_fresnel_hack(float n_dot_wi, float n_dot_wo) const noexcept;

    Layer layer_;

    float3 diffuse_color_;
    float3 f0_;
    float3 emission_;

    float metallic_;
    float alpha_;

    bool avoid_caustics_;
};

}  // namespace scene::material::substitute

#endif
