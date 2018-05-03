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
								   Layer const& layer) {
	float3 const reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, layer);
	float const  pdf = n_dot_wi * math::Pi_inv;

	SOFT_ASSERT(testing::check(reflection, float3(0.f), n_dot_wi, n_dot_wo, h_dot_wi, pdf, layer));

	return { reflection, pdf };
}

template<typename Layer>
float Isotropic::reflect(f_float3 wo, float n_dot_wo, Layer const& layer,
						 sampler::Sampler& sampler, bxdf::Sample& result) {
	float2 const s2d = sampler.generate_sample_2D();
	float3 const is = math::sample_hemisphere_cosine(s2d);
	float3 const wi = math::normalize(layer.tangent_to_world(is));

	float3 const h = math::normalize(wo + wi);
	float const h_dot_wi = clamp_dot(h, wi);

	float const n_dot_wi = layer.clamp_n_dot(wi);

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
float3 Isotropic::evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, Layer const& layer) {
//	float const f_D90 = 0.5f + (2.f * layer.roughness_) * (h_dot_wi * h_dot_wi);
//	float const fmo   = f_D90 - 1.f;

//	float const a = 1.f + fmo * math::pow5(1.f - n_dot_wi);
//	float const b = 1.f + fmo * math::pow5(1.f - n_dot_wo);

//	return (a * b) * (math::Pi_inv * layer.diffuse_color_);

	// More energy conserving variant
	float const energy_bias   = math::lerp(0.f, 0.5f, layer.roughness_);
	float const energy_factor = math::lerp(1.f, 1.f / 1.51f, layer.roughness_);

	float const f_D90 = energy_bias + (2.f * layer.roughness_) * (h_dot_wi * h_dot_wi);
	float const fmo	  = f_D90 - 1.f;

	float const a = 1.f + fmo * math::pow5(1.f - n_dot_wi);
	float const b = 1.f + fmo * math::pow5(1.f - n_dot_wo);

	return (a * b * energy_factor) * (math::Pi_inv * layer.diffuse_color_);
}

template<typename Layer>
bxdf::Result Isotropic_no_lambert::reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
											  Layer const& layer) {
	float3 const reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, layer);
	float const  pdf = n_dot_wi * math::Pi_inv;

	SOFT_ASSERT(testing::check(reflection, float3(0.f), n_dot_wi, n_dot_wo, h_dot_wi, pdf, layer));

	return { reflection, pdf };
}

template<typename Layer>
float Isotropic_no_lambert::reflect(f_float3 wo, float n_dot_wo, Layer const& layer,
									sampler::Sampler& sampler, bxdf::Sample& result) {
	float2 const s2d = sampler.generate_sample_2D();
	float3 const is = math::sample_hemisphere_cosine(s2d);
	float3 const wi = math::normalize(layer.tangent_to_world(is));

	float3 const h = math::normalize(wo + wi);
	float const h_dot_wi = clamp_dot(h, wi);

	float const n_dot_wi = layer.clamp_n_dot(wi);

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
									  Layer const& layer) {
	float const energy_factor = math::lerp(1.f, 1.f / 1.51f, layer.roughness_);

	float const fl = math::pow5(1.f - n_dot_wi);
	float const fv = math::pow5(1.f - n_dot_wo);
	float const rr = energy_factor * (2.f * layer.roughness_) * (h_dot_wi * h_dot_wi);

	// only the retro-reflection
	return rr * ((fl + fv) + (fl * fv) * (rr - 1.f)) * (math::Pi_inv * layer.diffuse_color_);
}

template<typename Layer>
bxdf::Result Isotropic_scaled_lambert::reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
												  Layer const& layer) {
	float3 const reflection = evaluate(h_dot_wi, n_dot_wi, n_dot_wo, layer);
	float const  pdf = n_dot_wi * math::Pi_inv;

	SOFT_ASSERT(testing::check(reflection, float3(0.f), n_dot_wi, n_dot_wo, h_dot_wi, pdf, layer));

	return { reflection, pdf };
}

template<typename Layer>
float Isotropic_scaled_lambert::reflect(f_float3 wo, float n_dot_wo, Layer const& layer,
										sampler::Sampler& sampler, bxdf::Sample& result) {
	float2 const s2d = sampler.generate_sample_2D();
	float3 const is = math::sample_hemisphere_cosine(s2d);
	float3 const wi = math::normalize(layer.tangent_to_world(is));

	float3 const h = math::normalize(wo + wi);
	float const h_dot_wi = clamp_dot(h, wi);

	float const n_dot_wi = layer.clamp_n_dot(wi);

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
										  Layer const& layer) {
	// More energy conserving variant
	float const energy_bias   = math::lerp(0.f, 0.5f, layer.roughness_);
	float const energy_factor = math::lerp(1.f, 1.f / 1.51f, layer.roughness_);

	float const f_D90 = (2.f * layer.roughness_) * (h_dot_wi * h_dot_wi);
	float const fmo	  = energy_bias + f_D90 - 1.f;

	float const fl = math::pow5(1.f - n_dot_wi);
	float const fv = math::pow5(1.f - n_dot_wo);
	float const a = 1.f + fmo * fl;
	float const b = 1.f + fmo * fv;

	// including lambert
	float const with = (a * b * energy_factor);

	float const rr = energy_factor * f_D90;

	// only the retro-reflection
	float const without = rr * ((fl + fv) + (fl * fv) * (rr - 1.f));

	float const rest = with - without;

	return (without + layer.lambert_scale * rest) * (math::Pi_inv * layer.diffuse_color_);
}

}
