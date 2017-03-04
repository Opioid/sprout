#pragma once

#include "ggx.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

namespace scene { namespace material { namespace ggx {

constexpr float Min_roughness = 0.01314f;

// constexpr float Min_a2 = 0.0000000299f;

constexpr float Min_a2 = Min_roughness * Min_roughness * Min_roughness * Min_roughness;

inline float clamp_roughness(float roughness) {
	return std::max(roughness, Min_roughness);
}

inline float map_roughness(float roughness) {
	return roughness * (1.f - Min_roughness) + Min_roughness;
}

inline float clamp_a2(float a2) {
	return std::max(a2, Min_a2);
}

inline float distribution_isotropic(float n_dot_h, float a2) {
	const float d = (n_dot_h * n_dot_h) * (a2 - 1.f) + 1.f;
	return a2 / (math::Pi * d * d);
}

inline float distribution_anisotropic(float n_dot_h, float x_dot_h, float y_dot_h,
									  float2 a2, float axy) {
	const float x = (x_dot_h * x_dot_h) / a2.v[0];
	const float y = (y_dot_h * y_dot_h) / a2.v[1];
	const float d = (x + y) + (n_dot_h * n_dot_h);

	return 1.f / ((math::Pi * axy) * (d * d));
}

inline float geometric_visibility_and_denominator(float n_dot_wi, float n_dot_wo, float a2) {
	// This is an optimized version that does the following in one step:
	//
	//    G_ggx(wi) * G_ggx(wo)
	// ---------------------------
	// 4 * dot(n, wi) * dot(n, wo)

	const float g_wo = n_dot_wo + std::sqrt((n_dot_wo - n_dot_wo * a2) * n_dot_wo + a2);
	const float g_wi = n_dot_wi + std::sqrt((n_dot_wi - n_dot_wi * a2) * n_dot_wi + a2);
	return 1.f / (g_wo * g_wi);
}

inline float3 microfacet(float d, float g, float3_p f, float n_dot_wi, float n_dot_wo) {
	return (d * g * f) / (4.f * n_dot_wi * n_dot_wo);
}

inline float G_ggx(float n_dot_v, float a2) {
	return (2.f * n_dot_v) / (n_dot_v + std::sqrt(a2 + (1.f - a2) * (n_dot_v * n_dot_v)));
}

inline float G_smith(float n_dot_wi, float n_dot_wo, float a2) {
	return G_ggx(n_dot_wi, a2) * G_ggx(n_dot_wo, a2);
}

template<typename Layer, typename Fresnel>
float3 Isotropic::reflection(float3_p h, float n_dot_wi, float n_dot_wo, float wo_dot_h,
							 const Layer& layer, const Fresnel& fresnel, float& pdf) {
	// Roughness zero will always have zero specular term (or worse NaN)
	SOFT_ASSERT(layer.a2_ >= Min_a2);

	const float n_dot_h = math::saturate(math::dot(layer.n_, h));

	const float a2 = layer.a2_;
	const float d = distribution_isotropic(n_dot_h, a2);
	const float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, a2);
	const float3 f = fresnel(wo_dot_h);

	pdf = (d * n_dot_h) / (4.f * wo_dot_h);
	const float3 result = d * g * f;

	SOFT_ASSERT(testing::check(result, h, n_dot_wi, n_dot_wo, wo_dot_h, pdf, layer));

	return result;
}

template<typename Layer, typename Fresnel>
float Isotropic::reflect(float3_p wo, float n_dot_wo, const Layer& layer, const Fresnel& fresnel,
						 sampler::Sampler& sampler, bxdf::Result& result) {
	// Roughness zero will always have zero specular term (or worse NaN)
	// For reflections we could do a perfect mirror,
	// but the decision is to always use a minimum roughness instead
	SOFT_ASSERT(layer.a2_ >= Min_a2);

	const float2 xi = sampler.generate_sample_2D();

	const float a2 = layer.a2_;
	const float n_dot_h = std::sqrt((1.f - xi.v[1]) / ((a2 - 1.f) * xi.v[0] + 1.f));

	const float sin_theta = std::sqrt(1.f - n_dot_h * n_dot_h);
	const float phi = (2.f * math::Pi) * xi.v[0];
	const float sin_phi = std::sin(phi);
	const float cos_phi = std::cos(phi);

	const float3 is = float3(sin_theta * cos_phi, sin_theta * sin_phi, n_dot_h);
	const float3 h = math::normalized(layer.tangent_to_world(is));

	const float wo_dot_h = math::clamp(math::dot(wo, h), 0.00001f, 1.f);

	const float3 wi = math::normalized(2.f * wo_dot_h * h - wo);

	const float n_dot_wi = layer.clamped_n_dot(wi);

	const float d = distribution_isotropic(n_dot_h, a2);
	const float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, a2);
	const float3 f = fresnel(wo_dot_h);

	result.pdf = (d * n_dot_h) / (4.f * wo_dot_h);
	result.reflection = d * g * f;
	result.wi = wi;
	result.h = h;
	result.h_dot_wi = wo_dot_h;
	result.type.clear_set(bxdf::Type::Glossy_reflection);

	SOFT_ASSERT(testing::check(result, wo, layer));

	return n_dot_wi;
}

template<typename Layer, typename Fresnel>
float3 Isotropic::refraction(float3_p wi, float3_p wo, float n_dot_wi,
							 float n_dot_wo, float /*n_dot_t*/, const Layer& layer,
							 const Fresnel& fresnel, float& pdf) {
	// Roughness zero will always have zero specular term (or worse NaN)
	SOFT_ASSERT(layer.a2_ >= Min_a2);

	const float3 h = math::normalized(wo + wi);
	const float wo_dot_h = math::clamp(math::dot(wo, h), 0.00001f, 1.f);
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
float Isotropic::refract(float3_p wo, float n_dot_wo, float n_dot_t, const Layer& layer,
						 const Fresnel& fresnel, sampler::Sampler& sampler, bxdf::Result& result) {
	// Roughness zero will always have zero specular term (or worse NaN)
	SOFT_ASSERT(layer.a2_ >= Min_a2);

	const float2 xi = sampler.generate_sample_2D();

	const float a2 = layer.a2_;
	const float n_dot_h = std::sqrt((1.f - xi.v[1]) / ((a2 - 1.f) * xi.v[1] + 1.f));

	const float sin_theta = std::sqrt(1.f - n_dot_h * n_dot_h);
	const float phi = (2.f * math::Pi) * xi.v[0];
	const float sin_phi = std::sin(phi);
	const float cos_phi = std::cos(phi);

	const float3 is = float3(sin_theta * cos_phi, sin_theta * sin_phi, n_dot_h);
	const float3 h = math::normalized(layer.tangent_to_world(is));

	const float wo_dot_h = math::clamp(math::dot(wo, h), 0.00001f, 1.f);

	const float3 wi = math::normalized((layer.eta_i_ * wo_dot_h - n_dot_t) * h - layer.eta_i_ * wo);

	const float n_dot_wi = layer.reversed_clamped_n_dot(wi);

	const float d = distribution_isotropic(n_dot_h, a2);
	const float g = G_smith(n_dot_wi, n_dot_wo, a2);
	const float3 f = fresnel(wo_dot_h);

	const float3 refraction = d * g * f;

	const float factor = (wo_dot_h * wo_dot_h) / (n_dot_wi * n_dot_wo);

	float denom = layer.ior_i_ * wo_dot_h + layer.ior_o_ * wo_dot_h;
	denom = denom * denom;

	result.pdf = (d * n_dot_h) / (4.f * wo_dot_h);

	const float ior_o_2 = layer.ior_o_ * layer.ior_o_;
	result.reflection = factor * ((ior_o_2 * refraction) / denom) * layer.color_;
	result.wi = wi;
	result.h = h;
	result.h_dot_wi = wo_dot_h;
	result.type.clear_set(bxdf::Type::Glossy_transmission);

	SOFT_ASSERT(testing::check(result, wo, layer));

	return n_dot_wi;
}

template<typename Layer, typename Fresnel>
float3 Isotropic::reflection(float3_p h, float n_dot_wi, float n_dot_wo, float wo_dot_h,
							 const Layer& layer, const Fresnel& fresnel,
							 float3& fresnel_result, float& pdf) {
	// Roughness zero will always have zero specular term (or worse NaN)
	SOFT_ASSERT(layer.a2_ >= Min_a2);

	const float n_dot_h = math::saturate(math::dot(layer.n_, h));

	const float a2 = layer.a2_;
	const float d = distribution_isotropic(n_dot_h, a2);
	const float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, a2);
	const float3 f = fresnel(wo_dot_h);

	fresnel_result = f;
	pdf = (d * n_dot_h) / (4.f * wo_dot_h);
	const float3 result = d * g * f;

	SOFT_ASSERT(testing::check(result, h, n_dot_wi, n_dot_wo, wo_dot_h, pdf, layer));

	return result;
}

template<typename Layer, typename Fresnel>
float Isotropic::reflect(float3_p wo, float n_dot_wo, const Layer& layer, const Fresnel& fresnel,
						 sampler::Sampler& sampler, float3& fresnel_result, bxdf::Result& result) {
	// Roughness zero will always have zero specular term (or worse NaN)
	// For reflections we could do a perfect mirror,
	// but the decision is to always use a minimum roughness instead
	SOFT_ASSERT(layer.a2_ >= Min_a2);

	const float2 xi = sampler.generate_sample_2D();

	const float a2 = layer.a2_;
	const float n_dot_h = std::sqrt((1.f - xi.v[1]) / ((a2 - 1.f) * xi.v[1] + 1.f));

	const float sin_theta = std::sqrt(1.f - n_dot_h * n_dot_h);
	const float phi = (2.f * math::Pi) * xi.v[0];
	const float sin_phi = std::sin(phi);
	const float cos_phi = std::cos(phi);

	const float3 is = float3(sin_theta * cos_phi, sin_theta * sin_phi, n_dot_h);
	const float3 h = math::normalized(layer.tangent_to_world(is));

	const float wo_dot_h = math::clamp(math::dot(wo, h), 0.00001f, 1.f);

	const float3 wi = math::normalized(2.f * wo_dot_h * h - wo);

	const float n_dot_wi = layer.clamped_n_dot(wi);

	const float d = distribution_isotropic(n_dot_h, a2);
	const float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, a2);
	const float3 f = fresnel(wo_dot_h);

	fresnel_result = f;

	result.pdf = (d * n_dot_h) / (4.f * wo_dot_h);
	result.reflection = d * g * f;
	result.wi = wi;
	result.h = h;
	result.h_dot_wi = wo_dot_h;
	result.type.clear_set(bxdf::Type::Glossy_reflection);

	SOFT_ASSERT(testing::check(result, wo, layer));

	return n_dot_wi;
}

template<typename Layer, typename Fresnel>
float3 Anisotropic::reflection(float3_p h, float n_dot_wi, float n_dot_wo, float wo_dot_h,
							   const Layer& layer, const Fresnel& fresnel, float &pdf) {
	const float n_dot_h = math::saturate(math::dot(layer.n_, h));

	const float x_dot_h  = math::dot(layer.t_, h);
	const float y_dot_h  = math::dot(layer.b_, h);

	const float d = distribution_anisotropic(n_dot_h, x_dot_h, y_dot_h, layer.a2_, layer.axy_);
	const float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, layer.axy_);
	const float3 f = fresnel(wo_dot_h);

	pdf = (d * n_dot_h) / (4.f * wo_dot_h);
	const float3 result = d * g * f;

	SOFT_ASSERT(testing::check(result, h, n_dot_wi, n_dot_wo, wo_dot_h, pdf, layer));

	return result;
}

template<typename Layer, typename Fresnel>
float Anisotropic::reflect(float3_p wo, float n_dot_wo, const Layer& layer, const Fresnel& fresnel,
						   sampler::Sampler& sampler, bxdf::Result& result) {
	const float2 xi = sampler.generate_sample_2D();

	const float phi = (2.f * math::Pi) * xi.v[0];
	const float sin_phi = std::sin(phi);
	const float cos_phi = std::cos(phi);

	const float t0 = std::sqrt(xi.v[1] / (1.f - xi.v[1]));
	const float3 t1 = layer.a_.v[0] * cos_phi * layer.t_ + layer.a_.v[1] * sin_phi * layer.b_;

	const float3 h = math::normalized(t0 * t1 + layer.n_);

	const float x_dot_h = math::dot(layer.t_, h);
	const float y_dot_h = math::dot(layer.b_, h);
	const float n_dot_h = math::dot(layer.n_, h);

	const float wo_dot_h = math::clamp(math::dot(wo, h), 0.00001f, 1.f);

	const float3 wi = math::normalized(2.f * wo_dot_h * h - wo);

	const float n_dot_wi = layer.clamped_n_dot(wi);

	const float d = distribution_anisotropic(n_dot_h, x_dot_h, y_dot_h, layer.a2_, layer.axy_);
	const float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, layer.axy_);
	const float3 f = fresnel(wo_dot_h);

	result.pdf = (d * n_dot_h) / (4.f * wo_dot_h);
	result.reflection = d * g * f;
	result.wi = wi;
	result.h = h;
	result.h_dot_wi = wo_dot_h;
	result.type.clear_set(bxdf::Type::Glossy_reflection);

	SOFT_ASSERT(testing::check(result, wo, layer));

	return n_dot_wi;
}

}}}
