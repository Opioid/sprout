#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::substitute {

template <typename Diffuse>
struct Base_closure {
    using Sampler = sampler::Sampler;

    void set(float3 const& color, float f0, float alpha, float metallic, bool avoid_caustics);

    template <bool Forward>
    bxdf::Result base_evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                               Layer const& layer) const;

    template <bool Forward>
    bxdf::Result base_evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                               Layer const& layer, float diffuse_factor) const;

    template <bool Forward>
    bxdf::Result pure_gloss_evaluate(float3 const& wi, float3 const& wo, float3 const& h,
                                     float wo_dot_h, Layer const& layer) const;

    void diffuse_sample(float3 const& wo, Layer const& layer, Sampler& sampler, RNG& rng,
                        bool avoid_caustics, bxdf::Sample& result) const;

    void diffuse_sample(float3 const& wo, Layer const& layer, float diffuse_factor,
                        Sampler& sampler, RNG& rng, bool avoid_caustics,
                        bxdf::Sample& result) const;

    void gloss_sample(float3 const& wo, Layer const& layer, Sampler& sampler, RNG& rng,
                      bxdf::Sample& result) const;

    void gloss_sample(float3 const& wo, Layer const& layer, float diffuse_factor, Sampler& sampler,
                      RNG& rng, bxdf::Sample& result) const;

    void pure_gloss_sample(float3 const& wo, Layer const& layer, Sampler& sampler, RNG& rng,
                           bxdf::Sample& result) const;

    float base_diffuse_fresnel_hack(float n_dot_wi, float n_dot_wo) const;

    float3 albedo_;
    float3 f0_;

    float metallic_;
    float alpha_;

    bool avoid_caustics_;
};

class Sample_base : public material::Sample {
  public:
    Layer layer_;
};

}  // namespace scene::material::substitute

#endif
