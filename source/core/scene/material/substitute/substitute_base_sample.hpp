#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_SAMPLE_HPP

#include "base/math/vector3.hpp"

namespace rnd {
class Generator;
}

using RNG = rnd::Generator;

namespace sampler {
class Sampler;
}

namespace scene::material {

namespace bxdf {
struct Result;
struct Sample;
}  // namespace bxdf

class Sample;

namespace substitute {

template <typename Diffuse>
struct Base_closure {
    using Sampler = sampler::Sampler;

    void set(float3_p color, float f0, float metallic);

    template <bool Forward>
    bxdf::Result base_evaluate(float3_p wi, float3_p wo, float3_p h, float wo_dot_h,
                               material::Sample const& sample) const;

    template <bool Forward>
    bxdf::Result base_evaluate(float3_p wi, float3_p wo, float3_p h, float wo_dot_h,
                               material::Sample const& sample, float diffuse_factor) const;

    template <bool Forward>
    bxdf::Result pure_gloss_evaluate(float3_p wi, float3_p wo, float3_p h, float wo_dot_h,
                                     material::Sample const& sample) const;

    void diffuse_sample(float3_p wo, material::Sample const& sample, Sampler& sampler, RNG& rng,
                        bxdf::Sample& result) const;

    void diffuse_sample(float3_p wo, material::Sample const& sample, float diffuse_factor,
                        Sampler& sampler, RNG& rng, bxdf::Sample& result) const;

    void gloss_sample(float3_p wo, material::Sample const& sample, Sampler& sampler, RNG& rng,
                      bxdf::Sample& result) const;

    void gloss_sample(float3_p wo, material::Sample const& sample, float diffuse_factor,
                      Sampler& sampler, RNG& rng, bxdf::Sample& result) const;

    void pure_gloss_sample(float3_p wo, material::Sample const& sample, Sampler& sampler, RNG& rng,
                           bxdf::Sample& result) const;

    float base_diffuse_fresnel_hack(float n_dot_wi, float n_dot_wo) const;

    float3 albedo_;
    float3 f0_;

    float metallic_;
};

}  // namespace substitute
}  // namespace scene::material

#endif
