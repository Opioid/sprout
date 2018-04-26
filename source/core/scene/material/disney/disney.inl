#pragma once

#include "disney.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "base/math/sampling/sampling.hpp"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

// Original disney description
// http://blog.selfshadow.com/publications/s2015-shading-course/burley/s2015_pbs_disney_bsdf_notes.pdf
// More energy conserving variant
// https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf

namespace scene::material::disney {

template<typename Layer>
bxdf::Result Isotropic::reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
								   const Layer& layer) {
	const float3 reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, layer);
	const float  pdf = n_dot_wi * math::Pi_inv;

	SOFT_ASSERT(testing::check(reflection, float3(0.f), n_dot_wi, n_dot_wo, h_dot_wi, pdf, layer));

	return { reflection, pdf };
}

template<typename Layer>
float Isotropic::reflect(f_float3 wo, float n_dot_wo, const Layer& layer,
						 sampler::Sampler& sampler, bxdf::Sample& result) {
	const float2 s2d = sampler.generate_sample_2D();
	const float3 is = math::sample_hemisphere_cosine(s2d);
	const float3 wi = math::normalize(layer.tangent_to_world(is));

	const float3 h = math::normalize(wo + wi);
	const float h_dot_wi = clamp_dot(h, wi);

	const float n_dot_wi = layer.clamp_n_dot(wi);

	result.reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, layer);
	result.wi = wi;
	result.h = h;
	result.pdf = n_dot_wi * math::Pi_inv;
	result.h_dot_wi = h_dot_wi;
	result.type.clear(bxdf::Type::Diffuse_reflection);

	SOFT_ASSERT(testing::check(result, wo, layer));

	return n_dot_wi;
}

template<typename Layer>
float3 Isotropic::evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, const Layer& layer) {
//	const float f_D90 = 0.5f + (2.f * layer.roughness_) * (h_dot_wi * h_dot_wi);
//	const float fmo   = f_D90 - 1.f;

//	const float a = 1.f + fmo * math::pow5(1.f - n_dot_wi);
//	const float b = 1.f + fmo * math::pow5(1.f - n_dot_wo);

//	return (a * b) * (math::Pi_inv * layer.diffuse_color_);

	// More energy conserving variant
	const float energy_bias   = math::lerp(0.f, 0.5f, layer.roughness_);
	const float energy_factor = math::lerp(1.f, 1.f / 1.51f, layer.roughness_);

	const float f_D90 = energy_bias + (2.f * layer.roughness_) * (h_dot_wi * h_dot_wi);
	const float fmo	  = f_D90 - 1.f;

	const float a = 1.f + fmo * math::pow5(1.f - n_dot_wi);
	const float b = 1.f + fmo * math::pow5(1.f - n_dot_wo);

	return (a * b * energy_factor) * (math::Pi_inv * layer.diffuse_color_);
}

template<typename Layer>
bxdf::Result Isotropic_no_lambert::reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
											  const Layer& layer) {
	const float3 reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, layer);
	const float  pdf = n_dot_wi * math::Pi_inv;

	SOFT_ASSERT(testing::check(reflection, float3(0.f), n_dot_wi, n_dot_wo, h_dot_wi, pdf, layer));

	return { reflection, pdf };
}

template<typename Layer>
float Isotropic_no_lambert::reflect(f_float3 wo, float n_dot_wo, const Layer& layer,
									sampler::Sampler& sampler, bxdf::Sample& result) {
	const float2 s2d = sampler.generate_sample_2D();
	const float3 is = math::sample_hemisphere_cosine(s2d);
	const float3 wi = math::normalize(layer.tangent_to_world(is));

	const float3 h = math::normalize(wo + wi);
	const float h_dot_wi = clamp_dot(h, wi);

	const float n_dot_wi = layer.clamp_n_dot(wi);

	result.reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, layer);
	result.wi = wi;
	result.h = h;
	result.pdf = n_dot_wi * math::Pi_inv;
	result.h_dot_wi = h_dot_wi;
	result.type.clear(bxdf::Type::Diffuse_reflection);

	SOFT_ASSERT(testing::check(result, wo, layer));

	return n_dot_wi;
}

template<typename Layer>
float3 Isotropic_no_lambert::evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo,
									  const Layer& layer) {
	const float energy_factor = math::lerp(1.f, 1.f / 1.51f, layer.roughness_);

	const float fl = math::pow5(1.f - n_dot_wi);
	const float fv = math::pow5(1.f - n_dot_wo);
	const float rr = energy_factor * (2.f * layer.roughness_) * (h_dot_wi * h_dot_wi);

	// only the retro-reflection
	return rr * ((fl + fv) + (fl * fv) * (rr - 1.f)) * (math::Pi_inv * layer.diffuse_color_);
}

template<typename Layer>
bxdf::Result Isotropic_scaled_lambert::reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
												  const Layer& layer) {
	const float3 reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, layer);
	const float  pdf = n_dot_wi * math::Pi_inv;

	SOFT_ASSERT(testing::check(reflection, float3(0.f), n_dot_wi, n_dot_wo, h_dot_wi, pdf, layer));

	return { reflection, pdf };
}

template<typename Layer>
float Isotropic_scaled_lambert::reflect(f_float3 wo, float n_dot_wo, const Layer& layer,
										sampler::Sampler& sampler, bxdf::Sample& result) {
	const float2 s2d = sampler.generate_sample_2D();
	const float3 is = math::sample_hemisphere_cosine(s2d);
	const float3 wi = math::normalize(layer.tangent_to_world(is));

	const float3 h = math::normalize(wo + wi);
	const float h_dot_wi = clamp_dot(h, wi);

	const float n_dot_wi = layer.clamp_n_dot(wi);

	result.reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, layer);
	result.wi = wi;
	result.h = h;
	result.pdf = n_dot_wi * math::Pi_inv;
	result.h_dot_wi = h_dot_wi;
	result.type.clear(bxdf::Type::Diffuse_reflection);

	SOFT_ASSERT(testing::check(result, wo, layer));

	return n_dot_wi;
}

template<typename Layer>
float3 Isotropic_scaled_lambert::evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo,
										  const Layer& layer) {
	// More energy conserving variant
	const float energy_bias   = math::lerp(0.f, 0.5f, layer.roughness_);
	const float energy_factor = math::lerp(1.f, 1.f / 1.51f, layer.roughness_);

	const float f_D90 = (2.f * layer.roughness_) * (h_dot_wi * h_dot_wi);
	const float fmo	  = energy_bias + f_D90 - 1.f;

	const float fl = math::pow5(1.f - n_dot_wi);
	const float fv = math::pow5(1.f - n_dot_wo);
	const float a = 1.f + fmo * fl;
	const float b = 1.f + fmo * fv;

	// including lambert
	const float with = (a * b * energy_factor);

	const float rr = energy_factor * f_D90;

	// only the retro-reflection
	const float without = rr * ((fl + fv) + (fl * fv) * (rr - 1.f));

	const float rest = with - without;

	return (without + layer.lambert_scale * rest) * (math::Pi_inv * layer.diffuse_color_);
}

}
