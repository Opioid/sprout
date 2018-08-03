#pragma once

#include "base/math/math.hpp"
#include "base/math/sampling/sampling.hpp"
#include "base/math/vector3.inl"
#include "oren_nayar.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"

#include "base/debug/assert.hpp"
#include "scene/material/material_test.hpp"

namespace scene::material::oren_nayar {

template <typename Layer>
float3 Isotropic::reflection(float3 const& wi, float3 const& wo, float n_dot_wi, float n_dot_wo,
                             Layer const& layer, float& pdf) {
    float on = f(wi, wo, n_dot_wi, n_dot_wo, layer.alpha2_);

    pdf           = n_dot_wi * math::Pi_inv;
    float3 result = on * layer.diffuse_color;

    SOFT_ASSERT(testing::check(result, wi, wo, pdf, layer));

    return result;
}

template <typename Layer>
float Isotropic::reflect(float3 const& wo, float n_dot_wo, Layer const& layer,
                         sampler::Sampler& sampler, bxdf::Sample& result) {
    float2 s2d = sampler.generate_sample_2D();

    float3 is = math::sample_hemisphere_cosine(s2d);
    float3 wi = math::normalize(layer.tangent_to_world(is));

    float const n_dot_wi = layer.clamp_n_dot(wi);

    float on = f(wi, wo, n_dot_wi, n_dot_wo, layer.alpha2_);

    result.pdf        = n_dot_wi * math::Pi_inv;
    result.reflection = on * layer.diffuse_color;
    result.wi         = wi;
    result.type.clear(bxdf::Type::Diffuse_reflection);

    SOFT_ASSERT(testing::check(result, wo, layer));

    return n_dot_wi;
}

inline float Isotropic::f(float3 const& wi, float3 const& wo, float n_dot_wi, float n_dot_wo,
                          float alpha2) {
    float wi_dot_wo = math::dot(wi, wo);

    float s = wi_dot_wo - n_dot_wi * n_dot_wo;

    float t;
    if (s >= 0.f) {
        t = std::min(1.f, n_dot_wi / n_dot_wo);
    } else {
        t = n_dot_wi;
    }

    float a = 1.f - 0.5f * (alpha2 / (alpha2 + 0.33f));
    float b = 0.45f * (alpha2 / (alpha2 + 0.09f));

    return math::Pi_inv * (a + b * s * t);
}

}  // namespace scene::material::oren_nayar
}
}
