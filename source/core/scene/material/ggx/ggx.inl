#ifndef SU_CORE_SCENE_MATERIAL_GGX_INL
#define SU_CORE_SCENE_MATERIAL_GGX_INL

#include "base/math/interpolated_function_2d.inl"
#include "base/math/interpolated_function_3d.inl"
#include "base/math/math.hpp"
#include "base/math/sampling.inl"
#include "base/math/sincos.hpp"
#include "base/math/vector3.inl"
#include "ggx.hpp"
#include "ggx_test.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material_sample_helper.hpp"

#include "base/debug/assert.hpp"
#include "scene/material/material_test.hpp"

namespace scene::material::ggx {

#include "ggx_integral.inl"

inline Interpolated_function_2D_N<float, E_size, E_size> const E_tex(&E[0][0]);

inline Interpolated_function_3D_N<float, E_s_size, E_s_size, E_s_size> const E_s_tex(&E_s[0][0][0]);

static inline float3 ilm_ep_conductor(float3_p f0, float n_dot_wo, float alpha, float metallic) {
    return 1.f + (metallic / E_tex(n_dot_wo, alpha) - 1.f) * f0;
}

static inline float ilm_ep_dielectric(float n_dot_wo, float alpha, float ior) {
    return 1.f / E_s_tex(n_dot_wo, alpha, ior - 1.f);
}

// static inline float ilm_ep_dielectric_diffuse(float n_dot_wo, float alpha, float ior)  {
//    return 1.f / E_sd_tex(n_dot_wo, alpha, ior - 1.f);
//}

inline float constexpr Min_roughness = 0.01314f;

inline float constexpr Min_alpha = Min_roughness * Min_roughness;

static inline float clamp_roughness(float roughness) {
    return std::max(roughness, Min_roughness);
}

static inline float map_roughness(float roughness) {
    return roughness * (1.f - Min_roughness) + Min_roughness;
}

static inline float distribution_isotropic(float n_dot_h, float a2) {
    float const d = (n_dot_h * n_dot_h) * (a2 - 1.f) + 1.f;
    return a2 / (Pi * d * d);
}

static inline float distribution_anisotropic(float n_dot_h, float x_dot_h, float y_dot_h,
                                             float2 a) {
    float2 const a2 = a * a;

    float const x = (x_dot_h * x_dot_h) / a2[0];
    float const y = (y_dot_h * y_dot_h) / a2[1];
    float const d = (x + y) + (n_dot_h * n_dot_h);

    return 1.f / (Pi * (a[0] * a[1]) * (d * d));
}

static inline float masking_shadowing_and_denominator(float n_dot_wi, float n_dot_wo, float a2) {
    // Un-correlated version
    // This is an optimized version that does the following in one step:
    //
    //    G_ggx(wi) * G_ggx(wo)
    // ---------------------------
    // 4 * dot(n, wi) * dot(n, wo)

    // Height-correlated version
    // https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
    // lambda_v = ( -1 + sqrt ( alphaG2 * (1 - NdotL2 ) / NdotL2 + 1)) * 0.5 f;
    // lambda_l = ( -1 + sqrt ( alphaG2 * (1 - NdotV2 ) / NdotV2 + 1)) * 0.5 f;
    // G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l );
    // V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0 f * NdotL * NdotV );

    // Optimized version
    // Caution: the "n_dot_wi *" and "n_dot_wo *" are explicitely inversed, this is not a mistake.
    float const g_wo = n_dot_wi * std::sqrt(a2 + (1.f - a2) * (n_dot_wo * n_dot_wo));
    float const g_wi = n_dot_wo * std::sqrt(a2 + (1.f - a2) * (n_dot_wi * n_dot_wi));

    return 0.5f / (g_wo + g_wi);
}

static inline float2 visibility_and_g1_wo(float n_dot_wi, float n_dot_wo, float alpha2) {
    //    float const t_wi = std::sqrt(alpha2 + (1.f - alpha2) * (n_dot_wi * n_dot_wi));
    //    float const t_wo = std::sqrt(alpha2 + (1.f - alpha2) * (n_dot_wo * n_dot_wo));

    Simd3f const n_dot(n_dot_wi, n_dot_wo);
    Simd3f const a2(alpha2);

    Simd3f const t = sqrt(a2 + (1.f - a2) * (n_dot * n_dot));

    float const t_wi = t.x();
    float const t_wo = t.y();

    return {0.5f / (n_dot_wi * t_wo + n_dot_wo * t_wi), t_wo + n_dot_wo};
}

// https://google.github.io/filament/Filament.html#listing_approximatedspecularv
static inline float masking_shadowing_and_denominator(float t_dot_wi, float t_dot_wo,
                                                      float b_dot_wi, float b_dot_wo,
                                                      float n_dot_wi, float n_dot_wo, float2 a) {
    float const g_wo = n_dot_wi * length(float3(a[0] * t_dot_wo, a[1] * b_dot_wo, n_dot_wo));
    float const g_wi = n_dot_wo * length(float3(a[0] * t_dot_wi, a[1] * b_dot_wi, n_dot_wi));

    return 0.5f / (g_wo + g_wi);
}

static inline float2 visibility_and_g1_wo(float t_dot_wi, float t_dot_wo, float b_dot_wi,
                                          float b_dot_wo, float n_dot_wi, float n_dot_wo,
                                          float2 a) {
    float const t_wo = length(float3(a[0] * t_dot_wo, a[1] * b_dot_wo, n_dot_wo));
    float const t_wi = length(float3(a[0] * t_dot_wi, a[1] * b_dot_wi, n_dot_wi));

    return {0.5f / (n_dot_wi * t_wo + n_dot_wo * t_wi), t_wo + n_dot_wo};
}

static inline float3 microfacet(float d, float g, float3_p f, float n_dot_wi, float n_dot_wo) {
    return (d * g * f) / (4.f * n_dot_wi * n_dot_wo);
}

static inline float G_ggx(float n_dot_v, float alpha2) {
    return (2.f * n_dot_v) / (n_dot_v + std::sqrt(alpha2 + (1.f - alpha2) * (n_dot_v * n_dot_v)));
}

static inline float G_smith(float n_dot_wi, float n_dot_wo, float alpha2) {
    return G_ggx(n_dot_wi, alpha2) * G_ggx(n_dot_wo, alpha2);
}

static inline float G_smith_correlated(float n_dot_wi, float n_dot_wo, float alpha2) {
    float const a = n_dot_wo * std::sqrt(alpha2 + (1.f - alpha2) * (n_dot_wi * n_dot_wi));
    float const b = n_dot_wi * std::sqrt(alpha2 + (1.f - alpha2) * (n_dot_wo * n_dot_wo));

    return (2.f * n_dot_wi * n_dot_wo) / (a + b);
}

static inline float pdf(float n_dot_h, float wo_dot_h, float d) {
    return (d * n_dot_h) / (4.f * wo_dot_h);
}

// This PDF is for the distribution of visible normals
// https://hal.archives-ouvertes.fr/hal-01509746/document
// https://hal.inria.fr/hal-00996995v2/document
static inline float pdf_visible(float n_dot_wo, float wo_dot_h, float d, float alpha2) {
    float const g1 = G_ggx(n_dot_wo, alpha2);

    return (g1 * wo_dot_h * d / n_dot_wo) / (4.f * wo_dot_h);
}

static inline float pdf_visible_refract(float n_dot_wo, float wo_dot_h, float d, float alpha2) {
    //    return (wo_dot_h * d);

    float const g1 = G_ggx(n_dot_wo, alpha2);

    return (g1 * wo_dot_h * d / n_dot_wo);
}

static inline float pdf_visible(float d, float g1_wo) {
    //	return (0.25f * g1_wo * d) / n_dot_wo;

    return (0.5f * d) / g1_wo;
}

template <typename Fresnel>
bxdf::Result Iso::reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
                             float alpha, Fresnel fresnel) {
    float3 fresnel_result;
    return reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha, fresnel, fresnel_result);
}

template <typename Fresnel>
bxdf::Result Iso::reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
                             float alpha, Fresnel fresnel, float3& fresnel_result) {
    float const alpha2 = alpha * alpha;

    float const  d = distribution_isotropic(n_dot_h, alpha2);
    float2 const g = visibility_and_g1_wo(n_dot_wi, n_dot_wo, alpha2);
    float3 const f = fresnel(wo_dot_h);

    fresnel_result = f;

    float3 const reflection = d * g[0] * f;

    float const pdf = pdf_visible(d, g[1]);

    SOFT_ASSERT(testing::check(reflection, n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, pdf));

    return {reflection, pdf};
}

template <typename Fresnel>
float Iso::reflect(float3_p wo, float n_dot_wo, float alpha, Fresnel fresnel, float2 xi,
                   Layer const& layer, bxdf::Sample& result) {
    float3 fresnel_result;
    return reflect(wo, n_dot_wo, alpha, fresnel, xi, layer, fresnel_result, result);
}

template <typename Fresnel>
float Iso::reflect(float3_p wo, float n_dot_wo, float alpha, Fresnel fresnel, float2 xi,
                   Layer const& layer, float3& fresnel_result, bxdf::Sample& result) {
    float        n_dot_h;
    float3 const h = Aniso::sample(wo, float2(alpha), xi, layer, n_dot_h);

    float const wo_dot_h = clamp_dot(wo, h);

    float3 const wi = normalize(2.f * wo_dot_h * h - wo);

    float const n_dot_wi = layer.clamp_n_dot(wi);

    float const alpha2 = alpha * alpha;

    float const  d = distribution_isotropic(n_dot_h, alpha2);
    float2 const g = visibility_and_g1_wo(n_dot_wi, n_dot_wo, alpha2);
    float3 const f = fresnel(wo_dot_h);

    fresnel_result = f;

    result.reflection = d * g[0] * f;
    result.wi         = wi;
    result.h          = h;
    result.pdf        = pdf_visible(d, g[1]);
    result.h_dot_wi   = wo_dot_h;
    //    result.type.clear(bxdf::Type::Glossy_reflection);
    result.type.clear(alpha <= Min_alpha ? bxdf::Type::Specular_reflection
                                         : bxdf::Type::Glossy_reflection);

    SOFT_ASSERT(check(result, wo, n_dot_wi, n_dot_wo, wo_dot_h, layer));

    return n_dot_wi;
}

// https://schuttejoe.github.io/post/disneybsdf/
// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf

template <typename Fresnel>
bxdf::Result Iso::refraction(float n_dot_wi, float n_dot_wo, float wi_dot_h, float wo_dot_h,
                             float n_dot_h, float alpha, IoR ior, Fresnel fresnel) {
    float const alpha2 = alpha * alpha;

    float const abs_wi_dot_h = clamp_abs(wi_dot_h);
    float const abs_wo_dot_h = clamp_abs(wo_dot_h);

    float const d = distribution_isotropic(n_dot_h, alpha2);

    float const g = G_smith_correlated(n_dot_wi, n_dot_wo, alpha2);

    float const cos_x = ior.eta_i > ior.eta_t ? abs_wi_dot_h : abs_wo_dot_h;

    float const f = 1.f - fresnel(cos_x);

    float const sqr_eta_t = ior.eta_t * ior.eta_t;

    float const factor = (abs_wi_dot_h * abs_wo_dot_h) / (n_dot_wi * n_dot_wo);

    float const denom = pow2(ior.eta_i * wo_dot_h + ior.eta_t * wi_dot_h);

    float const refraction = d * g * f;

    float const reflection = (factor * sqr_eta_t / denom) * refraction;

    float const pdf = pdf_visible_refract(n_dot_wo, abs_wo_dot_h, d, alpha2);

    return {float3(reflection), pdf * f * (abs_wi_dot_h * sqr_eta_t / denom)};
}

template <typename Fresnel>
float Iso::refract(float3_p wo, float n_dot_wo, float alpha, IoR ior, Fresnel fresnel, float2 xi,
                   Layer const& layer, bxdf::Sample& result) {
    float        n_dot_h;
    float3 const h = Aniso::sample(wo, float2(alpha), xi, layer, n_dot_h);

    float const wo_dot_h = clamp_dot(wo, h);

    float const eta = ior.eta_i / ior.eta_t;

    float const sint2 = (eta * eta) * (1.f - wo_dot_h * wo_dot_h);

    if (sint2 >= 1.f) {
        result.pdf = 0.f;
        return 0.f;
    }

    float const wi_dot_h = std::sqrt(1.f - sint2);

    float3 const wi = normalize((eta * wo_dot_h - wi_dot_h) * h - eta * wo);

    float const n_dot_wi = layer.clamp_abs_n_dot(wi);

    float const alpha2 = alpha * alpha;

    float const d = distribution_isotropic(n_dot_h, alpha2);
    float const g = G_smith_correlated(n_dot_wi, n_dot_wo, alpha2);

    float const cos_x = ior.eta_i > ior.eta_t ? wi_dot_h : wo_dot_h;

    float const f = 1.f - fresnel(cos_x);

    float const refraction = d * g * f;

    float const factor = (wi_dot_h * wo_dot_h) / (n_dot_wi * n_dot_wo);

    float const denom = pow2(ior.eta_i * wo_dot_h + ior.eta_t * wi_dot_h);

    float const sqr_eta_t = ior.eta_t * ior.eta_t;

    result.reflection = float3((factor * sqr_eta_t / denom) * refraction);
    result.wi         = wi;
    result.h          = h;

    float const pdf = pdf_visible_refract(n_dot_wo, wo_dot_h, d, alpha2);
    result.pdf      = pdf * (wi_dot_h * sqr_eta_t / denom);

    result.h_dot_wi = wi_dot_h;
    result.type.clear(alpha <= Min_alpha ? bxdf::Type::Specular_transmission
                                         : bxdf::Type::Glossy_transmission);

    SOFT_ASSERT(testing::check(result, wo, layer));

    return n_dot_wi;
}

inline float Iso::reflect(float3_p wo, float3_p h, float n_dot_wo, float n_dot_h, float wi_dot_h,
                          float wo_dot_h, float alpha, Layer const& layer, bxdf::Sample& result) {
    float3 const wi = normalize(2.f * wo_dot_h * h - wo);

    float const n_dot_wi = layer.clamp_n_dot(wi);

    float const alpha2 = alpha * alpha;

    float const  d = distribution_isotropic(n_dot_h, alpha2);
    float2 const g = visibility_and_g1_wo(n_dot_wi, n_dot_wo, alpha2);

    result.reflection = float3(d * g[0]);
    result.wi         = wi;
    result.h          = h;
    result.pdf        = pdf_visible(d, g[1]);
    result.h_dot_wi   = wi_dot_h;
    result.type.clear(alpha <= Min_alpha ? bxdf::Type::Specular_reflection
                                         : bxdf::Type::Glossy_reflection);

    SOFT_ASSERT(check(result, wo, n_dot_wi, n_dot_wo, wo_dot_h, layer));

    return n_dot_wi;
}

inline float Iso::refract(float3_p wo, float3_p h, float n_dot_wo, float n_dot_h, float wi_dot_h,
                          float wo_dot_h, float alpha, IoR ior, Layer const& layer,
                          bxdf::Sample& result) {
    float const eta = ior.eta_i / ior.eta_t;

    float const abs_wi_dot_h = clamp_abs(wi_dot_h);
    float const abs_wo_dot_h = clamp_abs(wo_dot_h);

    float3 const wi = normalize((eta * abs_wo_dot_h - abs_wi_dot_h) * h - eta * wo);

    float const n_dot_wi = layer.clamp_abs_n_dot(wi);

    float const alpha2 = alpha * alpha;

    float const d = distribution_isotropic(n_dot_h, alpha2);
    float const g = G_smith_correlated(n_dot_wi, n_dot_wo, alpha2);

    float const refraction = d * g;

    float const factor = (abs_wi_dot_h * abs_wo_dot_h) / (n_dot_wi * n_dot_wo);

    float const denom = math::pow2(ior.eta_i * wo_dot_h + ior.eta_t * wi_dot_h);

    float const sqr_eta_t = ior.eta_t * ior.eta_t;

    result.reflection = float3((factor * sqr_eta_t / denom) * refraction);
    result.wi         = wi;
    result.h          = h;

    float const pdf = pdf_visible_refract(n_dot_wo, abs_wo_dot_h, d, alpha2);
    result.pdf      = pdf * (abs_wi_dot_h * sqr_eta_t / denom);

    result.h_dot_wi = wi_dot_h;
    result.type.clear(alpha <= Min_alpha ? bxdf::Type::Specular_transmission
                                         : bxdf::Type::Glossy_transmission);

    return n_dot_wi;
}

template <typename Fresnel>
bxdf::Result Aniso::reflection(float3_p wi, float3_p wo, float3_p h, float n_dot_wi, float n_dot_wo,
                               float wo_dot_h, float2 alpha, Fresnel fresnel, Layer const& layer) {
    if (alpha[0] == alpha[1]) {
        float const n_dot_h = saturate(dot(layer.n_, h));

        return Iso::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha[0], fresnel);
    }

    float const n_dot_h = saturate(dot(layer.n_, h));

    float const x_dot_h = dot(layer.t_, h);
    float const y_dot_h = dot(layer.b_, h);

    float const d = distribution_anisotropic(n_dot_h, x_dot_h, y_dot_h, alpha);

    float const t_dot_wi = dot(layer.t_, wi);
    float const t_dot_wo = dot(layer.t_, wo);
    float const b_dot_wi = dot(layer.b_, wi);
    float const b_dot_wo = dot(layer.b_, wo);

    float2 const g = visibility_and_g1_wo(t_dot_wi, t_dot_wo, b_dot_wi, b_dot_wo, n_dot_wi,
                                          n_dot_wo, alpha);

    float3 const f = fresnel(wo_dot_h);

    float const  pdf        = pdf_visible(d, g[1]);
    float3 const reflection = d * g[0] * f;

    SOFT_ASSERT(testing::check(reflection, h, n_dot_wi, n_dot_wo, wo_dot_h, pdf, layer));

    return {reflection, pdf};
}

template <typename Fresnel>
float Aniso::reflect(float3_p wo, float n_dot_wo, float2 alpha, Fresnel fresnel, float2 xi,
                     Layer const& layer, bxdf::Sample& result) {
    if (alpha[0] == alpha[1]) {
        return Iso::reflect(wo, n_dot_wo, alpha[0], fresnel, xi, layer, result);
    }

    float        n_dot_h;
    float3 const h = sample(wo, alpha, xi, layer, n_dot_h);

    float const x_dot_h = dot(layer.t_, h);
    float const y_dot_h = dot(layer.b_, h);
    //   float const n_dot_h = dot(layer.n_, h);

    float const wo_dot_h = clamp_dot(wo, h);

    float3 const wi = normalize(2.f * wo_dot_h * h - wo);

    float const n_dot_wi = layer.clamp_n_dot(wi);

    float const d = distribution_anisotropic(n_dot_h, x_dot_h, y_dot_h, alpha);

    float const t_dot_wi = dot(layer.t_, wi);
    float const t_dot_wo = dot(layer.t_, wo);
    float const b_dot_wi = dot(layer.b_, wi);
    float const b_dot_wo = dot(layer.b_, wo);

    float2 const g = visibility_and_g1_wo(t_dot_wi, t_dot_wo, b_dot_wi, b_dot_wo, n_dot_wi,
                                          n_dot_wo, alpha);

    float3 const f = fresnel(wo_dot_h);

    result.reflection = d * g[0] * f;
    result.wi         = wi;
    result.h          = h;
    result.pdf        = pdf_visible(d, g[1]);
    result.h_dot_wi   = wo_dot_h;
    result.type.clear(bxdf::Type::Glossy_reflection);

    SOFT_ASSERT(testing::check(result, wo, layer));

    return n_dot_wi;
}

inline float3 Aniso::sample(float3_p wo, float2 alpha, float2 xi, Layer const& layer,
                            float& n_dot_h) {
    float3 const lwo = layer.world_to_tangent(wo);

    // stretch view
    float3 const v = normalize(float3(alpha[0] * lwo[0], alpha[1] * lwo[1], lwo[2]));

    // orthonormal basis
    float3 const cross_v_z = normalize(float3(v[1], -v[0], 0.f));  // == cross(v, [0, 0, 1])

    float3 const t1 = (v[2] < 0.9999f) ? cross_v_z : float3(1.f, 0.f, 0.f);
    float3 const t2 = float3(t1[1] * v[2], -t1[0] * v[2], t1[0] * v[1] - t1[1] * v[0]);

    // sample point with polar coordinates (r, phi)
    float const a   = 1.f / (1.f + v[2]);
    float const r   = std::sqrt(xi[0]);
    float const phi = (xi[1] < a) ? (xi[1] / a * Pi) : (Pi + (xi[1] - a) / (1.f - a) * Pi);

    auto const [sin_phi, cos_phi] = sincos(phi);

    float const p1 = r * cos_phi;
    float const p2 = r * sin_phi * ((xi[1] < a) ? 1.f : v[2]);

    // compute normal
    float3 m = p1 * t1 + p2 * t2 + std::sqrt(std::max(1.f - p1 * p1 - p2 * p2, 0.f)) * v;

    // unstretch
    m = normalize(float3(alpha[0] * m[0], alpha[1] * m[1], std::max(m[2], 0.f)));

    n_dot_h = clamp(m[2]);

    float3 const h = layer.tangent_to_world(m);

    return h;
}

}  // namespace scene::material::ggx

#endif
