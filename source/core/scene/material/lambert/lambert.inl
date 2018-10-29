#pragma once

#include "base/math/math.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "lambert.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"

#include "base/debug/assert.hpp"
#include "scene/material/material_test.hpp"

namespace scene::material::lambert {

inline bxdf::Result Isotropic::reflection(float3 const& color, float n_dot_wi,
                                          Layer const& layer) noexcept {
    float3 const reflection = math::Pi_inv * color;

    float const pdf = n_dot_wi * math::Pi_inv;

    (void)layer;
    //    SOFT_ASSERT(testing::check(reflection, float3::identity(), float3::identity(), pdf,
    //    layer));

    return {reflection, pdf};
}

inline float Isotropic::reflect(float3 const& color, Layer const& layer, sampler::Sampler& sampler,
                                bxdf::Sample& result) noexcept {
    float2 const s2d = sampler.generate_sample_2D();

    float3 const is = math::sample_hemisphere_cosine(s2d);

    float3 const wi = math::normalize(layer.tangent_to_world(is));

    float const n_dot_wi = layer.clamp_n_dot(wi);

    result.reflection = math::Pi_inv * color;
    result.wi         = wi;
    result.pdf        = n_dot_wi * math::Pi_inv;
    result.type.clear(bxdf::Type::Diffuse_reflection);

    SOFT_ASSERT(testing::check(result, float3::identity(), layer));

    return n_dot_wi;
}

}  // namespace scene::material::lambert
