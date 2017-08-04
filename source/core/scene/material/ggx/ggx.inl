#pragma once

#include "ggx.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample_helper.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "base/math/sincos.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

// #define EXPERIMENTAL_GGX

namespace scene { namespace material { namespace ggx {

constexpr float Min_roughness = 0.01314f;

// constexpr float Min_a2 = 0.0000000299f;

constexpr float Min_a2 = Min_roughness * Min_roughness * Min_roughness * Min_roughness;

static inline float clamp_roughness(float roughness) {
	return std::max(roughness, Min_roughness);
}

static inline float map_roughness(float roughness) {
	return roughness * (1.f - Min_roughness) + Min_roughness;
}

static inline float clamp_a2(float a2) {
	return std::max(a2, Min_a2);
}

static inline float distribution_isotropic(float n_dot_h, float a2) {
	const float d = (n_dot_h * n_dot_h) * (a2 - 1.f) + 1.f;
	return a2 / (math::Pi * d * d);
}

static inline float distribution_anisotropic(float n_dot_h, float x_dot_h, float y_dot_h,
											 float2 a2, float axy) {
	const float x = (x_dot_h * x_dot_h) / a2[0];
	const float y = (y_dot_h * y_dot_h) / a2[1];
	const float d = (x + y) + (n_dot_h * n_dot_h);

	return 1.f / ((math::Pi * axy) * (d * d));
}

static inline float geometric_visibility_and_denominator(float n_dot_wi, float n_dot_wo, float a2) {
	// Un-correlated version
	// This is an optimized version that does the following in one step:
	//
	//    G_ggx(wi) * G_ggx(wo)
	// ---------------------------
	// 4 * dot(n, wi) * dot(n, wo)

//	const float g_wo = n_dot_wo + std::sqrt((n_dot_wo - n_dot_wo * a2) * n_dot_wo + a2);
//	const float g_wi = n_dot_wi + std::sqrt((n_dot_wi - n_dot_wi * a2) * n_dot_wi + a2);
//	return 1.f / (g_wo * g_wi);

	// Correlated version
	// https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
	// lambda_v = ( -1 + sqrt ( alphaG2 * (1 - NdotL2 ) / NdotL2 + 1)) * 0.5 f;
	// lambda_l = ( -1 + sqrt ( alphaG2 * (1 - NdotV2 ) / NdotV2 + 1)) * 0.5 f;
	// G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l );
	// V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0 f * NdotL * NdotV );

	// Optimized version
	// Caution: the "n_dot_wi *" and "n_dot_wo *" are explicitely inversed, this is not a mistake.
	const float g_wo = n_dot_wi * std::sqrt((n_dot_wo - n_dot_wo * a2) * n_dot_wo + a2);
	const float g_wi = n_dot_wo * std::sqrt((n_dot_wi - n_dot_wi * a2) * n_dot_wi + a2);

	return 0.5f / (g_wo + g_wi);
}

static inline float3 microfacet(float d, float g, const float3& f, float n_dot_wi, float n_dot_wo) {
	return (d * g * f) / (4.f * n_dot_wi * n_dot_wo);
}

static inline float G_ggx(float n_dot_v, float a2) {
	return (2.f * n_dot_v) / (n_dot_v + std::sqrt(a2 + (1.f - a2) * (n_dot_v * n_dot_v)));
}

static inline float G_smith(float n_dot_wi, float n_dot_wo, float a2) {
	return G_ggx(n_dot_wi, a2) * G_ggx(n_dot_wo, a2);
}

static inline float G_smith_correlated(float n_dot_wi, float n_dot_wo, float a2) {
	const float a = n_dot_wo * std::sqrt(a2 + (1.f - a2) * (n_dot_wi * n_dot_wi));
	const float b = n_dot_wi * std::sqrt(a2 + (1.f - a2) * (n_dot_wo * n_dot_wo));

	return (2.f * n_dot_wi * n_dot_wo) / (a + b);
}

template<typename Layer, typename Fresnel>
float3 Isotropic::reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
							 const Layer& layer, const Fresnel& fresnel, float& pdf) {
	float3 fresnel_result;
	return reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, layer, fresnel, fresnel_result, pdf);
}


// https://hal.archives-ouvertes.fr/hal-01509746/document

// https://hal.inria.fr/hal-00996995v2/document

inline float distribution_visible(float n_dot_wi, float n_dot_wo, float wi_dot_h,
								  float n_dot_h, float a2) {
	float g1 = G_ggx(n_dot_wo, a2);

	float NdotL2 = n_dot_wi * n_dot_wi;
	float NdotV2 = n_dot_wo * n_dot_wo;


//	 float lambda_v = ( -1.f + std::sqrt ( a2 * (1.f - NdotL2 ) / NdotL2 + 1.f)) * 0.5f;
//	 float lambda_l = ( -1.f + std::sqrt ( a2 * (1.f - NdotV2 ) / NdotV2 + 1.f)) * 0.5f;
//	 float g1 = 1.f / (1.f + lambda_v + lambda_l );

//	 float g1 = 0.5f / (lambda_v);


	return g1 * wi_dot_h * distribution_isotropic(n_dot_h, a2) / n_dot_wo;
}


template<typename Layer, typename Fresnel>
float3 Isotropic::reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
							 const Layer& layer, const Fresnel& fresnel,
							 float3& fresnel_result, float& pdf) {
	// Roughness zero will always have zero specular term (or worse NaN)
	SOFT_ASSERT(layer.a2_ >= Min_a2);

	const float a2 = layer.a2_;
	const float d = distribution_isotropic(n_dot_h, a2);
	const float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, a2);
	const float3 f = fresnel(wo_dot_h);

	fresnel_result = f;
#ifndef EXPERIMENTAL_GGX
	pdf = (d * n_dot_h) / (4.f * wo_dot_h);
#else
	pdf = distribution_visible(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, a2) / (4.f * wo_dot_h);
#endif
	const float3 result = d * g * f;

	SOFT_ASSERT(testing::check(result, n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, pdf, layer));

	return result;
}

template<typename Layer, typename Fresnel>
float Isotropic::reflect(const float3& wo, float n_dot_wo, const Layer& layer,
						 const Fresnel& fresnel, sampler::Sampler& sampler, bxdf::Result& result) {
	float3 fresnel_result;
	return reflect(wo, n_dot_wo, layer, fresnel, sampler, fresnel_result, result);
}

#ifndef EXPERIMENTAL_GGX

template<typename Layer, typename Fresnel>
float Isotropic::reflect(const float3& wo, float n_dot_wo, const Layer& layer,
						 const Fresnel& fresnel, sampler::Sampler& sampler,
						 float3& fresnel_result, bxdf::Result& result) {
	// Roughness zero will always have zero specular term (or worse NaN)
	// For reflections we could do a perfect mirror,
	// but the decision is to always use a minimum roughness instead
	SOFT_ASSERT(layer.a2_ >= Min_a2);

	const float2 xi = sampler.generate_sample_2D();

	const float a2 = layer.a2_;
	const float n_dot_h_squared = (1.f - xi[1]) / ((a2 - 1.f) * xi[1] + 1.f);
	const float sin_theta = std::sqrt(1.f - n_dot_h_squared);
	const float n_dot_h   = std::sqrt(n_dot_h_squared);
	const float phi = (2.f * math::Pi) * xi[0];
//	const float sin_phi = std::sin(phi);
//	const float cos_phi = std::cos(phi);
	float sin_phi;
	float cos_phi;
	math::sincos(phi, sin_phi, cos_phi);

	const float3 lh = float3(sin_theta * cos_phi, sin_theta * sin_phi, n_dot_h);
	const float3 h = math::normalize(layer.tangent_to_world(lh));

	const float wo_dot_h = clamp_dot(wo, h);

	const float3 wi = math::normalize(2.f * wo_dot_h * h - wo);

	const float n_dot_wi = layer.clamp_n_dot(wi);

	const float d = distribution_isotropic(n_dot_h, a2);
	const float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, a2);
	const float3 f = fresnel(wo_dot_h);

	fresnel_result = f;

	result.reflection = d * g * f;
	result.wi = wi;
	result.h = h;
	result.pdf = (d * n_dot_h) / (4.f * wo_dot_h);
	result.h_dot_wi = wo_dot_h;
	result.type.clear_set(bxdf::Type::Glossy_reflection);

	SOFT_ASSERT(testing::check(result, wo, layer));

	return n_dot_wi;
}

#else

template<typename Layer, typename Fresnel>
float Isotropic::reflect(const float3& wo, float n_dot_wo, const Layer& layer,
						 const Fresnel& fresnel, sampler::Sampler& sampler,
						 float3& fresnel_result, bxdf::Result& result) {
	// Roughness zero will always have zero specular term (or worse NaN)
	// For reflections we could do a perfect mirror,
	// but the decision is to always use a minimum roughness instead
	SOFT_ASSERT(layer.a2_ >= Min_a2);

	const float2 xi = sampler.generate_sample_2D();

	const float a2 = layer.a2_;
	const float aa  = std::sqrt(a2);



	const float3 lwo = layer.world_to_tangent(wo);



	float U1 = xi[0];
	float U2 = xi[1];
	// stretch view
	float3 V = math::normalize(float3(aa * lwo[0], aa * lwo[1], lwo[2]));
	// orthonormal basis
	float3 T1 = (V[0] < 0.9999f) ? math::normalize(math::cross(V, float3(0.f,0.f,1.f))) : float3(1.f,0.f,0.f);
	float3 T2 = math::cross(T1, V);


	// sample point with polar coordinates (r, phi)
	float a = 1.f / (1.f + V[2]);
	float r = std::sqrt(U1);
	float phi = (U2<a) ? U2/a * math::Pi : math::Pi + (U2-a)/(1.f-a) * math::Pi;
	float P1 = r * std::cos(phi);
	float P2 = r * std::sin(phi)*((U2<a) ? 1.f : V[2]);
	// compute normal
	float3 N = P1*T1 + P2*T2 + std::sqrt(std::max(0.f, 1.f - P1*P1 - P2*P2))*V;
	// unstretch
	N = math::normalize(float3(aa*N[0], aa*N[1], std::max(0.f, N[2])));

	float n_dot_h = N[2];

	float3 h = layer.tangent_to_world(N);








	float wo_dot_h = clamp_abs_dot(wo, h);

	const float3 wi = math::normalize(2.f * wo_dot_h * h - wo);

	const float n_dot_wi = layer.clamp_abs_n_dot(wi);

	const float d = distribution_isotropic(n_dot_h, a2);
	const float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, a2);
	const float3 f = fresnel(wo_dot_h);

	fresnel_result = f;

	result.reflection = d * g * f;
	result.wi = wi;
	result.h = h;
	result.pdf = distribution_visible(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, a2) / (4.f * wo_dot_h);
//	result.pdf = (d * n_dot_h);// / (4.f * wo_dot_h);
//	result.pdf = pdfly(layer.world_to_tangent(wi), N, aa);
	result.h_dot_wi = wo_dot_h;
	result.type.clear_set(bxdf::Type::Glossy_reflection);

	SOFT_ASSERT(testing::check(result, wo, layer));


	return n_dot_wi;
}

#endif

template<typename Layer, typename Fresnel>
float3 Isotropic::refraction(const float3& wi, const float3& wo, float n_dot_wi,
							 float n_dot_wo, float /*n_dot_t*/, const Layer& layer,
							 const Fresnel& fresnel, float& pdf) {
	// Roughness zero will always have zero specular term (or worse NaN)
	SOFT_ASSERT(layer.a2_ >= Min_a2);

	const float3 h = math::normalize(wo + wi);
	const float wo_dot_h = clamp_dot(wo, h);
	const float n_dot_h  = math::saturate(math::dot(layer.n_, h));

	const float a2 = layer.a2_;
	const float d = distribution_isotropic(n_dot_h, a2);
	const float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, a2);
	const float3 f = fresnel(wo_dot_h);

	pdf = (d * n_dot_h) / (4.f * wo_dot_h);
	const float3 result = (d * g) * (f * layer.color);

	SOFT_ASSERT(testing::check(result, h, n_dot_wi, n_dot_wo, wo_dot_h, pdf, layer));

	return result;
}

template<typename Layer, typename Fresnel>
float Isotropic::refract(const float3& wo, float n_dot_wo, float n_dot_t, const Layer& layer,
						 const Fresnel& fresnel, sampler::Sampler& sampler, bxdf::Result& result) {
	return refract(wo, n_dot_wo, n_dot_t, layer, layer, fresnel, sampler, result);
}

template<typename Layer, typename IOR, typename Fresnel>
float Isotropic::refract(const float3& wo, float n_dot_wo, float n_dot_t,
						 const Layer& layer, const IOR& ior, const Fresnel& fresnel,
						 sampler::Sampler& sampler, bxdf::Result& result) {
	// Roughness zero will always have zero specular term (or worse NaN)
	SOFT_ASSERT(layer.a2_ >= Min_a2);

	const float2 xi = sampler.generate_sample_2D();

	const float a2 = layer.a2_;
	const float n_dot_h_squared = (1.f - xi[1]) / ((a2 - 1.f) * xi[1] + 1.f);
	const float sin_theta = std::sqrt(1.f - n_dot_h_squared);
	const float n_dot_h   = std::sqrt(n_dot_h_squared);
	const float phi = (2.f * math::Pi) * xi[0];
//	const float sin_phi = std::sin(phi);
//	const float cos_phi = std::cos(phi);
	float sin_phi;
	float cos_phi;
	math::sincos(phi, sin_phi, cos_phi);

	const float3 lh = float3(sin_theta * cos_phi, sin_theta * sin_phi, n_dot_h);
	const float3 h = math::normalize(layer.tangent_to_world(lh));

	const float wo_dot_h = clamp_dot(wo, h);

	const float3 wi = math::normalize((ior.eta_i_ * wo_dot_h - n_dot_t) * h - ior.eta_i_ * wo);

	const float n_dot_wi = layer.clamp_reverse_n_dot(wi);

	const float d = distribution_isotropic(n_dot_h, a2);
	const float g = G_smith_correlated(n_dot_wi, n_dot_wo, a2);
	const float3 f = float3(1.f) - fresnel(wo_dot_h);

	const float3 refraction = d * g * f;

	const float factor = (wo_dot_h * wo_dot_h) / (n_dot_wi * n_dot_wo);

	float denom = (ior.ior_i_ + ior.ior_o_) * wo_dot_h;
	denom = denom * denom;

	const float ior_o_2 = ior.ior_o_ * ior.ior_o_;
	result.reflection = factor * ((ior_o_2 * refraction) / denom);
	result.wi = wi;
	result.h = h;
	result.pdf = (d * n_dot_h) / (4.f * wo_dot_h);
	result.h_dot_wi = wo_dot_h;
	result.type.clear_set(bxdf::Type::Glossy_transmission);

	SOFT_ASSERT(testing::check(result, wo, layer));

	return n_dot_wi;
}

template<typename Layer, typename Fresnel>
float3 Anisotropic::reflection(const float3& h, float n_dot_wi, float n_dot_wo, float wo_dot_h,
							   const Layer& layer, const Fresnel& fresnel, float &pdf) {
	const float n_dot_h = math::saturate(math::dot(layer.n_, h));

	const float x_dot_h = math::dot(layer.t_, h);
	const float y_dot_h = math::dot(layer.b_, h);

	const float d = distribution_anisotropic(n_dot_h, x_dot_h, y_dot_h, layer.a2_, layer.axy_);
	const float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, layer.axy_);
	const float3 f = fresnel(wo_dot_h);

	pdf = (d * n_dot_h) / (4.f * wo_dot_h);
	const float3 result = d * g * f;

	SOFT_ASSERT(testing::check(result, h, n_dot_wi, n_dot_wo, wo_dot_h, pdf, layer));

	return result;
}

template<typename Layer, typename Fresnel>
float Anisotropic::reflect(const float3& wo, float n_dot_wo,
						   const Layer& layer, const Fresnel& fresnel,
						   sampler::Sampler& sampler, bxdf::Result& result) {
	const float2 xi = sampler.generate_sample_2D();

	const float phi = (2.f * math::Pi) * xi[0];
	const float sin_phi = std::sin(phi);
	const float cos_phi = std::cos(phi);

	const float t0 = std::sqrt(xi[1] / (1.f - xi[1]));
	const float3 t1 = layer.a_[0] * cos_phi * layer.t_ + layer.a_[1] * sin_phi * layer.b_;

	const float3 h = math::normalize(t0 * t1 + layer.n_);

	const float x_dot_h = math::dot(layer.t_, h);
	const float y_dot_h = math::dot(layer.b_, h);
	const float n_dot_h = math::dot(layer.n_, h);

	const float wo_dot_h = clamp_dot(wo, h);

	const float3 wi = math::normalize(2.f * wo_dot_h * h - wo);

	const float n_dot_wi = layer.clamp_n_dot(wi);

	const float d = distribution_anisotropic(n_dot_h, x_dot_h, y_dot_h, layer.a2_, layer.axy_);
	const float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, layer.axy_);
	const float3 f = fresnel(wo_dot_h);

	result.reflection = d * g * f;
	result.wi = wi;
	result.h = h;
	result.pdf = (d * n_dot_h) / (4.f * wo_dot_h);
	result.h_dot_wi = wo_dot_h;
	result.type.clear_set(bxdf::Type::Glossy_reflection);

	SOFT_ASSERT(testing::check(result, wo, layer));

	return n_dot_wi;
}

}}}
