#ifndef SU_CORE_SCENE_MATERIAL_GGX_INL
#define SU_CORE_SCENE_MATERIAL_GGX_INL

#include "base/math/math.hpp"
#include "base/math/sincos.hpp"
#include "base/math/vector3.inl"
#include "ggx.hpp"
#include "ggx_test.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material_sample_helper.hpp"

#include "base/math/sampling.inl"

#include "base/debug/assert.hpp"
#include "scene/material/material_test.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace scene::material::ggx {

constexpr float Min_roughness = 0.01314f;

constexpr float Min_alpha  = Min_roughness * Min_roughness;
constexpr float Min_alpha2 = Min_roughness * Min_roughness * Min_roughness * Min_roughness;

static inline float clamp_roughness(float roughness) noexcept {
    return std::max(roughness, Min_roughness);
}

static inline float map_roughness(float roughness) noexcept {
    return roughness * (1.f - Min_roughness) + Min_roughness;
}

static inline float distribution_isotropic(float n_dot_h, float alpha2) noexcept {
    float const d = (n_dot_h * n_dot_h) * (alpha2 - 1.f) + 1.f;
    return alpha2 / (math::Pi * d * d);
}

static inline float distribution_anisotropic(float n_dot_h, float x_dot_h, float y_dot_h,
                                             float2 alpha2, float axy) noexcept {
    float const x = (x_dot_h * x_dot_h) / alpha2[0];
    float const y = (y_dot_h * y_dot_h) / alpha2[1];
    float const d = (x + y) + (n_dot_h * n_dot_h);

    return 1.f / ((math::Pi * axy) * (d * d));
}

static inline float masking_shadowing_and_denominator(float n_dot_wi, float n_dot_wo,
                                                      float alpha2) noexcept {
    // Un-correlated version
    // This is an optimized version that does the following in one step:
    //
    //    G_ggx(wi) * G_ggx(wo)
    // ---------------------------
    // 4 * dot(n, wi) * dot(n, wo)

    //	float const g_wo = n_dot_wo + std::sqrt((n_dot_wo - n_dot_wo * alpha2) * n_dot_wo + alpha2);
    //	float const g_wi = n_dot_wi + std::sqrt((n_dot_wi - n_dot_wi * alpha2) * n_dot_wi + alpha2);
    //	return 1.f / (g_wo * g_wi);

    // Height-correlated version
    // https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
    // lambda_v = ( -1 + sqrt ( alphaG2 * (1 - NdotL2 ) / NdotL2 + 1)) * 0.5 f;
    // lambda_l = ( -1 + sqrt ( alphaG2 * (1 - NdotV2 ) / NdotV2 + 1)) * 0.5 f;
    // G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l );
    // V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0 f * NdotL * NdotV );

    // Optimized version
    // Caution: the "n_dot_wi *" and "n_dot_wo *" are explicitely inversed, this is not a mistake.
    float const g_wo = n_dot_wi * std::sqrt(alpha2 + (1.f - alpha2) * (n_dot_wo * n_dot_wo));
    float const g_wi = n_dot_wo * std::sqrt(alpha2 + (1.f - alpha2) * (n_dot_wi * n_dot_wi));

    return 0.5f / (g_wo + g_wi);
}

static inline float2 optimized_masking_shadowing_and_g1_wo(float n_dot_wi, float n_dot_wo,
                                                           float alpha2) noexcept {
    float const t_wo = std::sqrt(alpha2 + (1.f - alpha2) * (n_dot_wo * n_dot_wo));
    float const t_wi = std::sqrt(alpha2 + (1.f - alpha2) * (n_dot_wi * n_dot_wi));

    return {0.5f / (n_dot_wi * t_wo + n_dot_wo * t_wi), t_wo + n_dot_wo};
}

static inline float3 microfacet(float d, float g, float3 const& f, float n_dot_wi,
                                float n_dot_wo) noexcept {
    return (d * g * f) / (4.f * n_dot_wi * n_dot_wo);
}

static inline float G_ggx(float n_dot_v, float alpha2) noexcept {
    return (2.f * n_dot_v) / (n_dot_v + std::sqrt(alpha2 + (1.f - alpha2) * (n_dot_v * n_dot_v)));
}

static inline float G_smith(float n_dot_wi, float n_dot_wo, float alpha2) noexcept {
    return G_ggx(n_dot_wi, alpha2) * G_ggx(n_dot_wo, alpha2);
}

static inline float G_smith_correlated(float n_dot_wi, float n_dot_wo, float alpha2) {
    float const a = n_dot_wo * std::sqrt(alpha2 + (1.f - alpha2) * (n_dot_wi * n_dot_wi));
    float const b = n_dot_wi * std::sqrt(alpha2 + (1.f - alpha2) * (n_dot_wo * n_dot_wo));

    return (2.f * n_dot_wi * n_dot_wo) / (a + b);
}

static inline float pdf(float n_dot_h, float wo_dot_h, float d) noexcept {
    return (d * n_dot_h) / (4.f * wo_dot_h);
}

// This PDF is for the distribution of visible normals
// https://hal.archives-ouvertes.fr/hal-01509746/document
// https://hal.inria.fr/hal-00996995v2/document
static inline float pdf_visible(float n_dot_wo, float wo_dot_h, float d, float alpha2) noexcept {
    float const g1 = G_ggx(n_dot_wo, alpha2);

    return (g1 * wo_dot_h * d / n_dot_wo) / (4.f * wo_dot_h);
}

static inline float pdf_visible_refract(float n_dot_wo, float wo_dot_h, float d,
                                        float alpha2) noexcept {
    //    return (wo_dot_h * d);

    float const g1 = G_ggx(n_dot_wo, alpha2);

    return (g1 * wo_dot_h * d / n_dot_wo);
}

static inline float pdf_visible(float d, float g1_wo) noexcept {
    //	return (0.25f * g1_wo * d) / n_dot_wo;

    return (0.5f * d) / g1_wo;
}

static inline float stellar_pdf(float3 wi, float3 wh, float3 n, float alpha) noexcept {
    float  dot_wi_wh            = std::abs(math::dot(wi, wh));
    float3 wh_inv_scaled        = wh / alpha;
    float  dot_wh_wh_inv_scaled = math::dot(wh_inv_scaled, wh_inv_scaled);
    float3 wi_scaled            = alpha * wi;
    float  dot_wi_wi_scaled     = math::dot(wi_scaled, wi_scaled);
    return dot_wi_wh / ((0.5f * math::Pi) * alpha * dot_wh_wh_inv_scaled * dot_wh_wh_inv_scaled *
                        (std::abs(math::dot(wi, n)) + std::sqrt(dot_wi_wi_scaled)));
}

static inline float G1(float3 const& lw, float alpha) noexcept {
    //	return 1.f / (1.f + Lambda(w, n, alpha2));

    float3 const alpha1(alpha, alpha, 1.f);

    float const n_dot_w = std::abs(lw[2]);
    return 2.f * n_dot_w / (math::length(alpha1 * lw) + n_dot_w);
}

/*
static inline float G(float3 const& view, float3 const& light, float3 const& half,
                                          float3 const& lview, float3 const& llight,
                                          float3 const& n, float alpha) {
        float const dotLH = math::dot(light, half);
        float const dotVH = math::dot(view , half);


        bool rstuff = true;
        if (dotLH <= 0.f || dotVH <= 0.f) {
                rstuff = false;
        };



        float const abs_lvz = std::abs(lview[2]);
        float const abs_llz = std::abs(llight[2]);

        float3 const alpha1(alpha, alpha, 1.f);

        float const ra = (2.f * abs_lvz * abs_llz) / (math::length(alpha1 * lview) * abs_llz +
math::length(alpha1 * llight) * abs_lvz);

        float const abs_vz = std::abs(math::dot(view, n));
        float const abs_lz = std::abs(math::dot(light, n));

        float const alpha2 = alpha * alpha;
        float const a = abs_vz * std::sqrt(alpha2 + (1.f - alpha2) * (abs_lz * abs_lz));
        float const b = abs_lz * std::sqrt(alpha2 + (1.f - alpha2) * (abs_vz * abs_vz));

        float const rb = (2.f * abs_vz * abs_lz) / (a + b);

        return ra;
}
*/

template <typename Layer, typename Fresnel>
bxdf::Result Isotropic::reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
                                   Layer const& layer, Fresnel const& fresnel) noexcept {
    float3 fresnel_result;
    return reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, layer, fresnel, fresnel_result);
}

template <typename Layer, typename Fresnel>
bxdf::Result Isotropic::reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
                                   Layer const& layer, Fresnel const& fresnel,
                                   float3& fresnel_result) noexcept {
    float const alpha2 = layer.alpha_ * layer.alpha_;

    float const  d = distribution_isotropic(n_dot_h, alpha2);
    float2 const g = optimized_masking_shadowing_and_g1_wo(n_dot_wi, n_dot_wo, alpha2);
    float3 const f = fresnel(wo_dot_h);

    fresnel_result = f;

    float3 const reflection = d * g[0] * f;

    float const pdf = pdf_visible(d, g[1]);

    SOFT_ASSERT(testing::check(reflection, n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, pdf, layer));

    return {reflection, pdf};
}

template <typename Layer, typename Fresnel>
float Isotropic::reflect(float3 const& wo, float n_dot_wo, Layer const& layer,
                         Fresnel const& fresnel, sampler::Sampler& sampler,
                         bxdf::Sample& result) noexcept {
    float3 fresnel_result;
    return reflect(wo, n_dot_wo, layer, fresnel, sampler, fresnel_result, result);
}

template <typename Layer, typename Fresnel>
float Isotropic::reflect(float3 const& wo, float n_dot_wo, Layer const& layer,
                         Fresnel const& fresnel, sampler::Sampler& sampler, float3& fresnel_result,
                         bxdf::Sample& result) noexcept {
    float2 const xi = sampler.generate_sample_2D();

    float const alpha = layer.alpha_;

    float3 const lwo = layer.world_to_tangent(wo);

    // stretch view
    float3 const v = math::normalize(float3(alpha * lwo[0], alpha * lwo[1], lwo[2]));

    // orthonormal basis
    float3 const cross_v_z = float3(v[1], -v[0], 0.f);  // == cross(v, [0, 0, 1])

    float3 const t1 = (v[2] < 0.9999f) ? math::normalize(cross_v_z) : float3(1.f, 0.f, 0.f);
    float3 const t2 = float3(t1[1] * v[2], -t1[0] * v[2], t1[0] * v[1] - t1[1] * v[0]);

    // sample point with polar coordinates (r, phi)
    float const a   = 1.f / (1.f + v[2]);
    float const r   = std::sqrt(xi[0]);
    float const phi = (xi[1] < a) ? xi[1] / a * math::Pi
                                  : math::Pi + (xi[1] - a) / (1.f - a) * math::Pi;

    auto const [sin_phi, cos_phi] = math::sincos(phi);

    float const p1 = r * cos_phi;
    float const p2 = r * sin_phi * ((xi[1] < a) ? 1.f : v[2]);

    // compute normal
    float3 m = p1 * t1 + p2 * t2 + std::sqrt(std::max(1.f - p1 * p1 - p2 * p2, 0.f)) * v;

    // unstretch
    m = math::normalize(float3(alpha * m[0], alpha * m[1], std::max(m[2], 0.f)));

    float const n_dot_h = clamp(m[2]);

    float3 const h = layer.tangent_to_world(m);

    float const wo_dot_h = clamp_dot(wo, h);

    float3 const wi = math::normalize(2.f * wo_dot_h * h - wo);

    float const n_dot_wi = layer.clamp_n_dot(wi);

    float const alpha2 = alpha * alpha;

    float const  d = distribution_isotropic(n_dot_h, alpha2);
    float2 const g = optimized_masking_shadowing_and_g1_wo(n_dot_wi, n_dot_wo, alpha2);
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

    SOFT_ASSERT(check(result, wo, n_dot_wi, n_dot_wo, wo_dot_h, layer, xi));

    return n_dot_wi;
}

template <typename Layer, typename IOR, typename Fresnel>
float Isotropic::reflect_internally(float3 const& wo, float n_dot_wo, Layer const& layer,
                                    IOR const& ior, Fresnel const& fresnel,
                                    sampler::Sampler& sampler, bxdf::Sample& result) noexcept {
    float2 const xi = sampler.generate_sample_2D();

    float const alpha = layer.alpha_;

    float3 const lwo = layer.world_to_tangent(wo);

    // stretch view
    float3 const v = math::normalize(float3(alpha * lwo[0], alpha * lwo[1], lwo[2]));

    // orthonormal basis
    float3 const cross_v_z = float3(v[1], -v[0], 0.f);  // == cross(v, [0, 0, 1])

    float3 const t1 = (v[2] < 0.9999f) ? math::normalize(cross_v_z) : float3(1.f, 0.f, 0.f);
    float3 const t2 = float3(t1[1] * v[2], -t1[0] * v[2], t1[0] * v[1] - t1[1] * v[0]);

    // sample point with polar coordinates (r, phi)
    float const a   = 1.f / (1.f + v[2]);
    float const r   = std::sqrt(xi[0]);
    float const phi = (xi[1] < a) ? (xi[1] / a * math::Pi)
                                  : (math::Pi + (xi[1] - a) / (1.f - a) * math::Pi);

    auto const [sin_phi, cos_phi] = math::sincos(phi);

    float const p1 = r * cos_phi;
    float const p2 = r * sin_phi * ((xi[1] < a) ? 1.f : v[2]);

    // compute normal
    float3 m = p1 * t1 + p2 * t2 + std::sqrt(std::max(1.f - p1 * p1 - p2 * p2, 0.f)) * v;

    // unstretch
    m = math::normalize(float3(alpha * m[0], alpha * m[1], std::max(m[2], 0.f)));

    float const n_dot_h = clamp(m[2]);

    float3 const h = layer.tangent_to_world(m);

    float const wo_dot_h = clamp_dot(wo, h);

    float const eta = ior.eta_i / ior.eta_t;

    float const sint2 = (eta * eta) * (1.f - wo_dot_h * wo_dot_h);

    float const wi_dot_h = std::sqrt(1.f - sint2);

    float3 const wi = math::normalize(2.f * wo_dot_h * h - wo);

    float const n_dot_wi = layer.clamp_n_dot(wi);

    float const alpha2 = alpha * alpha;

    float const  d = distribution_isotropic(n_dot_h, alpha2);
    float2 const g = optimized_masking_shadowing_and_g1_wo(n_dot_wi, n_dot_wo, alpha2);

    float const cos_x = ior.eta_i > ior.eta_t ? wi_dot_h : wo_dot_h;

    float const f = (sint2 >= 1.f) ? 1.f : fresnel(cos_x);

    result.reflection = float3(d * g[0] * f);
    result.wi         = wi;
    result.h          = h;
    result.pdf        = pdf_visible(d, g[1]);
    result.h_dot_wi   = wi_dot_h;
    result.type.clear(alpha <= Min_alpha ? bxdf::Type::Specular_reflection
                                         : bxdf::Type::Glossy_reflection);

    SOFT_ASSERT(check(result, wo, n_dot_wi, n_dot_wo, wo_dot_h, layer, xi));

    return n_dot_wi;
}

template <typename Layer, typename IoR, typename Fresnel>
bxdf::Result Isotropic::refraction(float n_dot_wi, float n_dot_wo, float wi_dot_h, float wo_dot_h,
                                   float n_dot_h, Layer const& layer, IoR const& ior,
                                   Fresnel const& fresnel) noexcept {
    float const alpha2 = layer.alpha_ * layer.alpha_;

    //    float const  d = distribution_isotropic(n_dot_h, alpha2);
    //    float2 const g = optimized_masking_shadowing_and_g1_wo(n_dot_wi, n_dot_wo, alpha2);
    //    float3 const f = float3(1.f) - fresnel(wo_dot_h);

    float const d = distribution_isotropic(n_dot_h, alpha2);
    float const g = G_smith_correlated(n_dot_wi, n_dot_wo, alpha2);

    float const cos_x = ior.eta_i > ior.eta_t ? wi_dot_h : wo_dot_h;

    float const f = 1.f - fresnel(cos_x);

    float const refraction = d * g * f;

    float const factor = (wi_dot_h * wo_dot_h) / (n_dot_wi * n_dot_wo);

    float const denom = math::pow2(ior.eta_i * wi_dot_h + ior.eta_t * wo_dot_h);

    float const sqr_eta_t = ior.eta_t * ior.eta_t;

    float const eta = ior.eta_i / ior.eta_t;

    float const sqr_eta = 1.f;  // eta * eta;

    float const reflection = sqr_eta * (factor * sqr_eta_t / denom) * refraction;

    //	result.pdf = pdf_visible(n_dot_wo, wo_dot_h, d, alpha2);// * (wi_dot_h * sqr_ior_i / denom);
    float const pdf = pdf_visible_refract(n_dot_wo, wo_dot_h, d, alpha2) *
                      (wi_dot_h * sqr_eta_t / denom);

    SOFT_ASSERT(testing::check(reflection, n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, pdf, layer));

    //  std::cout << "refraction:" << std::endl;
    //	std::cout << "h: " << h << std::endl;
    //	std::cout << "d: " << d << std::endl;
    //  std::cout << "n_dot_wi: " << n_dot_wi << std::endl;
    //  std::cout << "n_dot_wo: " << n_dot_wo << std::endl;
    //  std::cout << "wo_dot_h: " << wo_dot_h << std::endl;
    //  std::cout << "wi_dot_h: " << wi_dot_h << std::endl;
    //  std::cout << "n_dot_h: " << n_dot_h << std::endl;
    //	std::cout << "alpha2: " << alpha2 << std::endl;
    //	std::cout << "fresnel: " << f << std::endl;
    //	std::cout << "factor: " << factor << std::endl;
    //	std::cout << "eta_t_2: " << eta_t_2 << std::endl;
    //	std::cout << "refraction: " << refraction << std::endl;
    //	std::cout << "denom: " << denom << std::endl;

    return {float3(reflection), pdf};
}

// Refraction details according to
// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf

template <typename Layer, typename IoR, typename Fresnel>
float Isotropic::refract(float3 const& wo, float n_dot_wo, Layer const& layer, IoR const& ior,
                         Fresnel const& fresnel, sampler::Sampler& sampler,
                         bxdf::Sample& result) noexcept {
    float2 const xi = sampler.generate_sample_2D();

    float const alpha = layer.alpha_;

    float3 const lwo = layer.world_to_tangent(wo);

    // stretch view
    float3 const v = math::normalize(float3(alpha * lwo[0], alpha * lwo[1], lwo[2]));

    // orthonormal basis
    float3 const cross_v_z = float3(v[1], -v[0], 0.f);  // == cross(v, [0, 0, 1])

    float3 const t1 = (v[2] < 0.9999f) ? math::normalize(cross_v_z) : float3(1.f, 0.f, 0.f);
    float3 const t2 = float3(t1[1] * v[2], -t1[0] * v[2], t1[0] * v[1] - t1[1] * v[0]);

    // sample point with polar coordinates (r, phi)
    float const a   = 1.f / (1.f + v[2]);
    float const r   = std::sqrt(xi[0]);
    float const phi = (xi[1] < a) ? (xi[1] / a * math::Pi)
                                  : (math::Pi + (xi[1] - a) / (1.f - a) * math::Pi);

    auto const [sin_phi, cos_phi] = math::sincos(phi);

    float const p1 = r * cos_phi;
    float const p2 = r * sin_phi * ((xi[1] < a) ? 1.f : v[2]);

    // compute normal
    float3 m = p1 * t1 + p2 * t2 + std::sqrt(std::max(1.f - p1 * p1 - p2 * p2, 0.f)) * v;

    // unstretch
    m = math::normalize(float3(alpha * m[0], alpha * m[1], std::max(m[2], 0.f)));

    float const n_dot_h = clamp(m[2]);

    float3 const h = layer.tangent_to_world(m);

    float const wo_dot_h = clamp_dot(wo, h);

    float const eta = ior.eta_i / ior.eta_t;

    float const sint2 = (eta * eta) * (1.f - wo_dot_h * wo_dot_h);

    if (sint2 >= 1.f) {
        result.pdf = 0.f;
        return 0.f;
    }

    float const wi_dot_h = std::sqrt(1.f - sint2);

    float3 const wi = math::normalize((eta * wo_dot_h - wi_dot_h) * h - eta * wo);

    float const n_dot_wi = layer.clamp_reverse_n_dot(wi);

    float const alpha2 = alpha * alpha;

    float const d = distribution_isotropic(n_dot_h, alpha2);
    float const g = G_smith_correlated(n_dot_wi, n_dot_wo, alpha2);

    float const cos_x = ior.eta_i > ior.eta_t ? wi_dot_h : wo_dot_h;

    float const f = 1.f - fresnel(cos_x);

    float const refraction = d * g * f;

    float const factor = (wi_dot_h * wo_dot_h) / (n_dot_wi * n_dot_wo);

    float const denom = math::pow2(ior.eta_i * wi_dot_h + ior.eta_t * wo_dot_h);

    float const sqr_eta_t = ior.eta_t * ior.eta_t;

    float const sqr_eta = 1.f;  // eta * eta;

    result.reflection = float3(sqr_eta * (factor * sqr_eta_t / denom) * refraction);
    result.wi         = wi;
    result.h          = h;
    float const pdf   = pdf_visible_refract(n_dot_wo, wo_dot_h, d, alpha2);

    //    cosBeta  = dot(wh, view);
    //    cosAlpha = dot(wh, light);
    //    return d.pdf(view, wh) * sqr(ior_o) * abs(cosAlpha) / sqr(ior_i * cosBeta + ior_o *
    //    cosAlpha);

    result.pdf      = pdf * (wi_dot_h * sqr_eta_t / denom);
    result.h_dot_wi = wi_dot_h;
    result.type.clear(alpha <= Min_alpha ? bxdf::Type::Specular_transmission
                                         : bxdf::Type::Glossy_transmission);

    SOFT_ASSERT(testing::check(result, wo, layer));

    //      std::cout << "refract:" << std::endl;
    //      std::cout << "h: " << h << std::endl;
    //      std::cout << "d: " << d << std::endl;
    //      std::cout << "n_dot_wi: " << n_dot_wi << std::endl;
    //      std::cout << "n_dot_wo: " << n_dot_wo << std::endl;
    //      std::cout << "wo_dot_h: " << wo_dot_h << std::endl;
    //      std::cout << "wi_dot_h: " << wi_dot_h << std::endl;
    //      std::cout << "n_dot_h: " << n_dot_h << std::endl;
    //    	std::cout << "alpha2: " << alpha2 << std::endl;
    //    	std::cout << "fresnel: " << f << std::endl;
    //    	std::cout << "factor: " << factor << std::endl;
    //    	std::cout << "eta_t_2: " << eta_t_2 << std::endl;
    //    	std::cout << "refraction: " << refraction << std::endl;
    //    	std::cout << "denom: " << denom << std::endl;

    return n_dot_wi;
}

template <typename Layer, typename Fresnel>
bxdf::Result Anisotropic::reflection(float3 const& h, float n_dot_wi, float n_dot_wo,
                                     float wo_dot_h, Layer const& layer,
                                     Fresnel const& fresnel) noexcept {
    float const n_dot_h = math::saturate(math::dot(layer.n_, h));

    float const x_dot_h = math::dot(layer.t_, h);
    float const y_dot_h = math::dot(layer.b_, h);

    float const  d = distribution_anisotropic(n_dot_h, x_dot_h, y_dot_h, layer.alpha2_, layer.axy_);
    float const  g = masking_shadowing_and_denominator(n_dot_wi, n_dot_wo, layer.axy_);
    float3 const f = fresnel(wo_dot_h);

    float const  pdf        = (d * n_dot_h) / (4.f * wo_dot_h);
    float3 const reflection = d * g * f;

    SOFT_ASSERT(testing::check(reflection, h, n_dot_wi, n_dot_wo, wo_dot_h, pdf, layer));

    return {reflection, pdf};
}

template <typename Layer, typename Fresnel>
float Anisotropic::reflect(float3 const& wo, float n_dot_wo, Layer const& layer,
                           Fresnel const& fresnel, sampler::Sampler& sampler,
                           bxdf::Sample& result) noexcept {
    float2 const xi = sampler.generate_sample_2D();

    float const phi     = (2.f * math::Pi) * xi[0];
    float const sin_phi = std::sin(phi);
    float const cos_phi = std::cos(phi);

    float const  t0 = std::sqrt(xi[1] / (1.f - xi[1]));
    float3 const t1 = layer.a_[0] * cos_phi * layer.t_ + layer.a_[1] * sin_phi * layer.b_;

    float3 const h = math::normalize(t0 * t1 + layer.n_);

    float const x_dot_h = math::dot(layer.t_, h);
    float const y_dot_h = math::dot(layer.b_, h);
    float const n_dot_h = math::dot(layer.n_, h);

    float const wo_dot_h = clamp_dot(wo, h);

    float3 const wi = math::normalize(2.f * wo_dot_h * h - wo);

    float const n_dot_wi = layer.clamp_n_dot(wi);

    float const  d = distribution_anisotropic(n_dot_h, x_dot_h, y_dot_h, layer.alpha2_, layer.axy_);
    float const  g = masking_shadowing_and_denominator(n_dot_wi, n_dot_wo, layer.axy_);
    float3 const f = fresnel(wo_dot_h);

    result.reflection = d * g * f;
    result.wi         = wi;
    result.h          = h;
    result.pdf        = (d * n_dot_h) / (4.f * wo_dot_h);
    result.h_dot_wi   = wo_dot_h;
    result.type.clear(bxdf::Type::Glossy_reflection);

    SOFT_ASSERT(testing::check(result, wo, layer));

    return n_dot_wi;
}

}  // namespace scene::material::ggx

#endif
