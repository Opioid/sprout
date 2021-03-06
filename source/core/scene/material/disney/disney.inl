#ifndef SU_CORE_SCENE_MATERIAL_DISNEY_INL
#define SU_CORE_SCENE_MATERIAL_DISNEY_INL

#include "base/math/math.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "disney.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample_helper.hpp"

#include "base/debug/assert.hpp"
#include "scene/material/material_test.hpp"

// Original disney description
// http://blog.selfshadow.com/publications/s2015-shading-course/burley/s2015_pbs_disney_bsdf_notes.pdf
// More energy conserving variant
// https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf

namespace scene::material::disney {

inline bxdf::Result Iso::reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo, float alpha,
                                    float3_p color) {
    float3 const reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, alpha, color);

    float const pdf = n_dot_wi * Pi_inv;

    //   SOFT_ASSERT(testing::check(reflection, float3(0.f), n_dot_wi, n_dot_wo, h_dot_wi, pdf,
    //   layer));

    return {reflection, pdf};
}

inline float Iso::reflect(float3_p wo, float n_dot_wo, Layer const& layer, float alpha,
                          float3_p color, float2 xi, bxdf::Sample& result) {
    float3 const is = sample_hemisphere_cosine(xi);
    float3 const wi = normalize(layer.tangent_to_world(is));

    float3 const h = normalize(wo + wi);

    float const h_dot_wi = clamp_dot(h, wi);

    float const n_dot_wi = layer.clamp_n_dot(wi);

    result.reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, alpha, color);
    result.wi         = wi;
    result.h          = h;
    result.pdf        = n_dot_wi * Pi_inv;
    result.h_dot_wi   = h_dot_wi;
    result.type.clear(bxdf::Type::Diffuse_reflection);

    SOFT_ASSERT(testing::check(result, wo, layer));

    return n_dot_wi;
}

inline float3 Iso::evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, float alpha,
                            float3_p color) {
    //	float const f_D90 = 0.5f + (2.f * layer.roughness_) * (h_dot_wi * h_dot_wi);
    //	float const fmo   = f_D90 - 1.f;

    //	float const a = 1.f + fmo * pow5(1.f - n_dot_wi);
    //	float const b = 1.f + fmo * pow5(1.f - n_dot_wo);

    //	return (a * b) * (Pi_inv * layer.albedo_);

    // More energy conserving variant
    float const energy_bias   = lerp(0.f, 0.5f, alpha);
    float const energy_factor = lerp(1.f, 1.f / 1.53f, alpha);

    float const f_D90 = energy_bias + (2.f * alpha) * (h_dot_wi * h_dot_wi);
    float const fmo   = f_D90 - 1.f;

    float const a = 1.f + fmo * pow5(1.f - n_dot_wi);
    float const b = 1.f + fmo * pow5(1.f - n_dot_wo);

    return (a * b * energy_factor) * (Pi_inv * color);
}

inline bxdf::Result Isotropic_no_lambert::reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
                                                     float alpha, float3_p color) {
    float3 const reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, alpha, color);

    float const pdf = n_dot_wi * Pi_inv;

    //   SOFT_ASSERT(testing::check(reflection, float3(0.f), n_dot_wi, n_dot_wo, h_dot_wi, pdf,
    //   layer));

    return {reflection, pdf};
}

inline float Isotropic_no_lambert::reflect(float3_p wo, float n_dot_wo, Layer const& layer,
                                           float alpha, float3_p color, float2 xi,
                                           bxdf::Sample& result) {
    float3 const is = sample_hemisphere_cosine(xi);
    float3 const wi = normalize(layer.tangent_to_world(is));

    float3 const h = normalize(wo + wi);

    float const h_dot_wi = clamp_dot(h, wi);

    float const n_dot_wi = layer.clamp_n_dot(wi);

    result.reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, alpha, color);
    result.wi         = wi;
    result.h          = h;
    result.pdf        = n_dot_wi * Pi_inv;
    result.h_dot_wi   = h_dot_wi;
    result.type.clear(bxdf::Type::Diffuse_reflection);

    SOFT_ASSERT(testing::check(result, wo, layer));

    return n_dot_wi;
}

inline float3 Isotropic_no_lambert::evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo,
                                             float alpha, float3_p color) {
    float const energy_factor = lerp(1.f, 1.f / 1.53f, alpha);

    float const fl = pow5(1.f - n_dot_wi);
    float const fv = pow5(1.f - n_dot_wo);
    float const rr = energy_factor * (2.f * alpha) * (h_dot_wi * h_dot_wi);

    // only the retro-reflection
    return rr * ((fl + fv) + (fl * fv) * (rr - 1.f)) * (Pi_inv * color);
}

}  // namespace scene::material::disney

#endif
