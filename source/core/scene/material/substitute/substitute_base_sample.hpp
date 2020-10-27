#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::substitute {

template <typename Diffuse>
struct Base_closure {
    using Sampler = sampler::Sampler;

    void set(float3 const& color, float f0, float metallic, bool avoid_caustics);

    template <bool Forward>
    bxdf::Result base_evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                               material::Sample const& sample) const;

    template <bool Forward>
    bxdf::Result base_evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                               material::Sample const& sample, float diffuse_factor) const;

    template <bool Forward>
    bxdf::Result pure_gloss_evaluate(float3 const& wi, float3 const& wo, float3 const& h,
                                     float wo_dot_h, Sample const& sample) const;

    void diffuse_sample(float3 const& wo, material::Sample const& sample, Sampler& sampler,
                        RNG& rng, bool avoid_caustics, bxdf::Sample& result) const;

    void diffuse_sample(float3 const& wo, material::Sample const& sample, float diffuse_factor,
                        Sampler& sampler, RNG& rng, bool avoid_caustics,
                        bxdf::Sample& result) const;

    void gloss_sample(float3 const& wo, material::Sample const& sample, Sampler& sampler, RNG& rng,
                      bxdf::Sample& result) const;

    void gloss_sample(float3 const& wo, material::Sample const& sample, float diffuse_factor,
                      Sampler& sampler, RNG& rng, bxdf::Sample& result) const;

    void pure_gloss_sample(float3 const& wo, material::Sample const& sample, Sampler& sampler,
                           RNG& rng, bxdf::Sample& result) const;

    float base_diffuse_fresnel_hack(float n_dot_wi, float n_dot_wo) const;

    float3 albedo_;
    float3 f0_;

    float metallic_;

    bool avoid_caustics_;
};

}  // namespace scene::material::substitute

#endif
