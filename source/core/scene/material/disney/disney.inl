#pragma once

#include "disney.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "base/math/sampling/sampling.hpp"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

namespace scene { namespace material { namespace disney {

template<typename Layer>
float3 Isotropic::reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
							 const Layer& layer, float& pdf) {
	pdf = n_dot_wi * math::Pi_inv;
	const float3 result = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, layer);

	SOFT_ASSERT(testing::check(result, float3(0.f), n_dot_wi, n_dot_wo, h_dot_wi, pdf, layer));

	return result;
}

template<typename Layer>
float Isotropic::reflect(const float3& wo, float n_dot_wo, const Layer& layer,
						 sampler::Sampler& sampler, bxdf::Result& result) {
	const float2 s2d = sampler.generate_sample_2D();

	const float3 is = math::sample_hemisphere_cosine(s2d);
	const float3 wi = math::normalized(layer.tangent_to_world(is));

	const float3 h = math::normalized(wo + wi);
	const float h_dot_wi = math::clamp(math::dot(h, wi), 0.00001f, 1.f);

	const float n_dot_wi = layer.clamped_n_dot(wi);

	result.pdf = n_dot_wi * math::Pi_inv;
	result.reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, layer);
	result.wi = wi;
	result.h = h;
	result.h_dot_wi = h_dot_wi;
	result.type.clear_set(bxdf::Type::Diffuse_reflection);

	SOFT_ASSERT(testing::check(result, wo, layer));

	return n_dot_wi;
}

template<typename Layer>
float3 Isotropic::evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, const Layer& layer) {
	const float f_D90 = 0.5f + (2.f * layer.roughness_) * (h_dot_wi * h_dot_wi);
	const float fmo = f_D90 - 1.f;

	const float a = 1.f + fmo * math::pow5(1.f - n_dot_wi);
	const float b = 1.f + fmo * math::pow5(1.f - n_dot_wo);

	return (a * b) * (math::Pi_inv * layer.diffuse_color_);
}

template<typename Layer>
float3 Isotropic_no_lambert::reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
										const Layer& layer, float& pdf) {
	pdf = n_dot_wi * math::Pi_inv;
	const float3 result = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, layer);

	SOFT_ASSERT(testing::check(result, float3(0.f), n_dot_wi, n_dot_wo, h_dot_wi, pdf, layer));

	return result;
}

template<typename Layer>
float Isotropic_no_lambert::reflect(const float3& wo, float n_dot_wo, const Layer& layer,
									sampler::Sampler& sampler, bxdf::Result& result) {
	const float2 s2d = sampler.generate_sample_2D();

	const float3 is = math::sample_hemisphere_cosine(s2d);
	const float3 wi = math::normalized(layer.tangent_to_world(is));

	const float3 h = math::normalized(wo + wi);
	const float h_dot_wi = math::clamp(math::dot(h, wi), 0.00001f, 1.f);

	const float n_dot_wi = layer.clamped_n_dot(wi);

	result.pdf = n_dot_wi * math::Pi_inv;
	result.reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, layer);
	result.wi = wi;
	result.h = h;
	result.h_dot_wi = h_dot_wi;
	result.type.clear_set(bxdf::Type::Diffuse_reflection);

	SOFT_ASSERT(testing::check(result, wo, layer));

	return n_dot_wi;
}

template<typename Layer>
float3 Isotropic_no_lambert::evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo,
									  const Layer& layer) {
	const float fl = math::pow5(1.f - n_dot_wi);
	const float fv = math::pow5(1.f - n_dot_wo);
	const float rr = (2.f * layer.roughness_) * (h_dot_wi * h_dot_wi);

	return rr * ((fl + fv) + (fl * fv) * (rr - 1.f)) * (math::Pi_inv * layer.diffuse_color_);
}

}}}
