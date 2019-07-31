#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::substitute {

template <typename Diffuse>
struct Base_closure {
    using Sampler = sampler::Sampler;

    void set(float3 const& color, float3 const& radiance, float f0, float alpha,
             float metallic) noexcept;

    void set(float3 const& f0, float3 const& a, float alpha) noexcept;

    template <bool Forward>
    bxdf::Result base_evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                               Layer const& layer, bool avoid_caustics) const noexcept;

    template <bool Forward>
    bxdf::Result pure_gloss_evaluate(float3 const& wi, float3 const& wo, float3 const& h,
                                     float wo_dot_h, Layer const& layer, bool avoid_caustics) const
        noexcept;

    void diffuse_sample(float3 const& wo, Layer const& layer, Sampler& sampler, bool avoid_caustics,
                        bxdf::Sample& result) const noexcept;

    void gloss_sample(float3 const& wo, Layer const& layer, Sampler& sampler,
                      bxdf::Sample& result) const noexcept;

    void pure_gloss_sample(float3 const& wo, Layer const& layer, Sampler& sampler,
                           bxdf::Sample& result) const noexcept;

    float base_diffuse_fresnel_hack(float n_dot_wi, float n_dot_wo) const noexcept;

    float3 diffuse_color_;
    float3 f0_;
    float3 a_;
    float3 emission_;

    float metallic_;
    float alpha_;
};

class Sample_base : public material::Sample {
  public:
    using Sampler = sampler::Sampler;

    Layer const& base_layer() const noexcept override final;

    Layer layer_;

    bool avoid_caustics_;
};

}  // namespace scene::material::substitute

#endif
