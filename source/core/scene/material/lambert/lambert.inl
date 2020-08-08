#ifndef SU_CORE_SCENE_MATERIAL_LAMBERT_INL
#define SU_CORE_SCENE_MATERIAL_LAMBERT_INL

#include "base/math/math.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "lambert.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"

#include "base/debug/assert.hpp"
#include "scene/material/material_test.hpp"

namespace scene::material::lambert {

inline bxdf::Result Isotropic::reflection(float3 const& color, float n_dot_wi, Layer const& layer) {
    float3 const reflection = Pi_inv * color;

    float const pdf = n_dot_wi * Pi_inv;

    (void)layer;
    //    SOFT_ASSERT(testing::check(reflection, float3(0.f), float3(0.f), pdf,
    //    layer));

    return {reflection, pdf};
}

inline float Isotropic::reflect(float3 const& color, Layer const& layer, sampler::Sampler& sampler,
                                rnd::Generator& rng, bxdf::Sample& result) {
    float2 const s2d = sampler.generate_sample_2D(rng);

    float3 const is = sample_hemisphere_cosine(s2d);

    float3 const wi = normalize(layer.tangent_to_world(is));

    float const n_dot_wi = layer.clamp_n_dot(wi);

    result.reflection = Pi_inv * color;
    result.wi         = wi;
    result.pdf        = n_dot_wi * Pi_inv;
    result.type.clear(bxdf::Type::Diffuse_reflection);

    SOFT_ASSERT(testing::check(result, float3(0.f), layer));

    return n_dot_wi;
}

}  // namespace scene::material::lambert

#endif
