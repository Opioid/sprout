#pragma once

#include "ggx.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "base/math/math.hpp"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

namespace scene { namespace material { namespace ggx {

template<typename Layer, typename Fresnel>
float3 Isotropic::evaluate(float3_p wi, float3_p wo, float n_dot_wi, float n_dot_wo,
						   const Layer& layer, const Fresnel& fresnel, float& pdf) {
	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == layer.a2) {
		pdf = 0.f;
		return math::float3_identity;
	}

	float3 h = math::normalized(wo + wi);

	float wo_dot_h = math::clamp(math::dot(wo, h), 0.00001f, 1.f);
	float n_dot_h  = math::saturate(math::dot(layer.n, h));

	float clamped_a2 = clamp_a2(layer.a2);
	float d = distribution_isotropic(n_dot_h, clamped_a2);
	float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, clamped_a2);
	float3 f = fresnel(wo_dot_h);

	pdf = d * n_dot_h / (4.f * wo_dot_h);
	float3 result = d * g * f;

	SOFT_ASSERT(testing::check(result, wi, wo, pdf, layer));

	return result;
}

template<typename Layer, typename Fresnel>
float Isotropic::sample(float3_p wo, float n_dot_wo, const Layer& layer, const Fresnel& fresnel,
						sampler::Sampler& sampler, bxdf::Result& result) {
	if (0.f == layer.a2) {
		constexpr float n_dot_h = 1.f;

		float wo_dot_h = n_dot_wo;

		float3 wi = math::normalized((2.f * wo_dot_h) * layer.n - wo);

		float d = distribution_isotropic(n_dot_h, Min_a2);
		float g = geometric_visibility_and_denominator(n_dot_wo, n_dot_wo, Min_a2);
		float3 f = fresnel(wo_dot_h);

		result.pdf = d * n_dot_h / (4.f * wo_dot_h);
		result.reflection = d * g * f;
		result.wi = wi;
		result.type.clear_set(bxdf::Type::Specular_reflection);

		SOFT_ASSERT(testing::check(result, wo, layer));

		return n_dot_wo;
	} else {
		float2 xi = sampler.generate_sample_2D();

		float clamped_a2 = clamp_a2(layer.a2);
		float n_dot_h = std::sqrt((1.f - xi.y) / ((clamped_a2 - 1.f) * xi.y + 1.f));

		float sin_theta = std::sqrt(1.f - n_dot_h * n_dot_h);
		float phi = 2.f * math::Pi * xi.x;
		float sin_phi = std::sin(phi);
		float cos_phi = std::cos(phi);

		float3 is = float3(sin_theta * cos_phi, sin_theta * sin_phi, n_dot_h);
		float3 h = math::normalized(layer.tangent_to_world(is));

		float wo_dot_h = math::clamp(math::dot(wo, h), 0.00001f, 1.f);

		float3 wi = math::normalized((2.f * wo_dot_h) * h - wo);

		float n_dot_wi = layer.clamped_n_dot(wi);

		float d = distribution_isotropic(n_dot_h, clamped_a2);
		float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, clamped_a2);
		float3 f = fresnel(wo_dot_h);

		result.pdf = d * n_dot_h / (4.f * wo_dot_h);
		result.reflection = d * g * f;
		result.wi = wi;
		result.type.clear_set(bxdf::Type::Glossy_reflection);

		SOFT_ASSERT(testing::check(result, wo, layer));

		return n_dot_wi;
	}
}

template<typename Layer, typename Fresnel>
float3 Isotropic::evaluate(float3_p wi, float3_p wo, float n_dot_wi, float n_dot_wo,
						   const Layer& layer, const Fresnel& fresnel,
						   float3& fresnel_result, float& pdf) {
	float3 h = math::normalized(wo + wi);

	float wo_dot_h = math::clamp(math::dot(wo, h), 0.00001f, 1.f);

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == layer.a2) {
		fresnel_result = fresnel(wo_dot_h);
		pdf = 0.f;
		return math::float3_identity;
	}

	float n_dot_h  = math::saturate(math::dot(layer.n, h));

	float clamped_a2 = clamp_a2(layer.a2);
	float d = distribution_isotropic(n_dot_h, clamped_a2);
	float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, clamped_a2);
	float3 f = fresnel(wo_dot_h);

	fresnel_result = f;
	pdf = d * n_dot_h / (4.f * wo_dot_h);
	float3 result = d * g * f;

	SOFT_ASSERT(testing::check(result, wi, wo, pdf, layer));

	return result;
}

template<typename Layer, typename Fresnel>
float Isotropic::sample(float3_p wo, float n_dot_wo, const Layer& layer, const Fresnel& fresnel,
						sampler::Sampler& sampler, float3& fresnel_result, bxdf::Result& result) {
	if (0.f == layer.a2) {
		constexpr float n_dot_h = 1.f;

		float wo_dot_h = n_dot_wo;

		float3 wi = math::normalized((2.f * wo_dot_h) * layer.n - wo);

		float d = distribution_isotropic(n_dot_h, Min_a2);
		float g = geometric_visibility_and_denominator(n_dot_wo, n_dot_wo, Min_a2);
		float3 f = fresnel(wo_dot_h);

		fresnel_result = f;

		result.pdf = d * n_dot_h / (4.f * wo_dot_h);
		result.reflection = d * g * f;
		result.wi = wi;
		result.type.clear_set(bxdf::Type::Specular_reflection);

		SOFT_ASSERT(testing::check(result, wo, layer));

		return n_dot_wo;
	} else {
		float2 xi = sampler.generate_sample_2D();

		float clamped_a2 = clamp_a2(layer.a2);
		float n_dot_h = std::sqrt((1.f - xi.y) / ((clamped_a2 - 1.f) * xi.y + 1.f));

		float sin_theta = std::sqrt(1.f - n_dot_h * n_dot_h);
		float phi = 2.f * math::Pi * xi.x;
		float sin_phi = std::sin(phi);
		float cos_phi = std::cos(phi);

		float3 is = float3(sin_theta * cos_phi, sin_theta * sin_phi, n_dot_h);
		float3 h = math::normalized(layer.tangent_to_world(is));

		float wo_dot_h = math::clamp(math::dot(wo, h), 0.00001f, 1.f);

		float3 wi = math::normalized((2.f * wo_dot_h) * h - wo);

		float n_dot_wi = layer.clamped_n_dot(wi);

		float d = distribution_isotropic(n_dot_h, clamped_a2);
		float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, clamped_a2);
		float3 f = fresnel(wo_dot_h);

		fresnel_result = f;

		result.pdf = d * n_dot_h / (4.f * wo_dot_h);
		result.reflection = d * g * f;
		result.wi = wi;
		result.type.clear_set(bxdf::Type::Glossy_reflection);

		SOFT_ASSERT(testing::check(result, wo, layer));

		return n_dot_wi;
	}
}

template<typename Layer, typename Fresnel>
float3 Anisotropic::evaluate(float3_p wi, float3_p wo, float n_dot_wi, float n_dot_wo,
							 const Layer& layer, const Fresnel& fresnel, float &pdf) {
	float3 h = math::normalized(wo + wi);

	float n_dot_h  = math::saturate(math::dot(layer.n, h));

	float x_dot_h  = math::dot(layer.t, h);
	float y_dot_h  = math::dot(layer.b, h);

	float wo_dot_h = math::clamp(math::dot(wo, h), 0.00001f, 1.f);

	float d = distribution_anisotropic(n_dot_h, x_dot_h, y_dot_h, layer.a2, layer.axy);
	float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, layer.axy);
	float3 f = fresnel(wo_dot_h);

	pdf = d * n_dot_h / (4.f * wo_dot_h);
	float3 result = d * g * f;

	SOFT_ASSERT(testing::check(result, wi, wo, pdf, layer));

	return result;
}

template<typename Layer, typename Fresnel>
float Anisotropic::sample(float3_p wo, float n_dot_wo, const Layer& layer, const Fresnel& fresnel,
						  sampler::Sampler& sampler, bxdf::Result& result) {
	float2 xi = sampler.generate_sample_2D();

	float phi = 2.f * math::Pi * xi.x;
	float sin_phi = std::sin(phi);
	float cos_phi = std::cos(phi);

	float t0 = std::sqrt(xi.y / (1.f - xi.y));
	float3 t1 = layer.a.x * cos_phi * layer.t + layer.a.y * sin_phi * layer.b;

	float3 h = math::normalized(t0 * t1 + layer.n);

	float x_dot_h = math::dot(layer.t, h);
	float y_dot_h = math::dot(layer.b, h);
	float n_dot_h = math::dot(layer.n, h);

	float wo_dot_h = std::max(math::dot(wo, h), 0.00001f);

	float3 wi = math::normalized((2.f * wo_dot_h) * h - wo);

	float n_dot_wi = layer.clamped_n_dot(wi);

	float d = distribution_anisotropic(n_dot_h, x_dot_h, y_dot_h, layer.a2, layer.axy);
	float g = geometric_visibility_and_denominator(n_dot_wi, n_dot_wo, layer.axy);
	float3 f = fresnel(wo_dot_h);

	result.pdf = d * n_dot_h / (4.f * wo_dot_h);
	result.reflection = d * g * f;
	result.wi = wi;
	result.type.clear_set(bxdf::Type::Glossy_reflection);

	SOFT_ASSERT(testing::check(result, wo, layer));

	return n_dot_wi;
}

inline float distribution_isotropic(float n_dot_h, float a2) {
	float d = n_dot_h * n_dot_h * (a2 - 1.f) + 1.f;
	return a2 / (math::Pi * d * d);
}

inline float distribution_anisotropic(float n_dot_h, float x_dot_h, float y_dot_h,
									  float2 a2, float axy) {
	float x = (x_dot_h * x_dot_h) / a2.x;
	float y = (y_dot_h * y_dot_h) / a2.y;
	float d = (x + y + n_dot_h * n_dot_h);

	return 1.f / (math::Pi * axy * d * d);
}

inline float geometric_visibility_and_denominator(float n_dot_wi, float n_dot_wo, float a2) {
	// this is an optimized version that does the following in one step:
	//
	//    G_ggx(wi) * G_ggx(wo)
	// ---------------------------
	// 4 * dot(n, wi) * dot(n, wo)

	float g_wo = n_dot_wo + std::sqrt((n_dot_wo - n_dot_wo * a2) * n_dot_wo + a2);
	float g_wi = n_dot_wi + std::sqrt((n_dot_wi - n_dot_wi * a2) * n_dot_wi + a2);
	return 1.f / (g_wo * g_wi);
}

inline float clamp_a2(float a2) {
	return std::max(a2, Min_a2);
}

}}}
