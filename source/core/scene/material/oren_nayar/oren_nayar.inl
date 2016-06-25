#pragma once

#include "oren_nayar.hpp"
#include "scene/material/bxdf.hpp"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sampling.inl"
\
namespace scene { namespace material { namespace oren_nayar {

template<typename Layer>
float3 Isotropic::evaluate(float3_p wi, float n_dot_wi, float n_dot_wo,
						   const Sample& sample, const Layer& layer, float& pdf) {
	float on = f(wi, n_dot_wi, n_dot_wo, sample, layer);

	pdf = n_dot_wi * math::Pi_inv;
	return on * layer.diffuse_color;
}

template<typename Layer>
float Isotropic::importance_sample(float n_dot_wo, const Sample& sample, const Layer& layer,
								   sampler::Sampler& sampler, bxdf::Result& result) {
	float2 s2d = sampler.generate_sample_2D();

	float3 is = math::sample_hemisphere_cosine(s2d);
	float3 wi = math::normalized(layer.tangent_to_world(is));

	float n_dot_wi = layer.clamped_n_dot(wi);

	float on = f(wi, n_dot_wi, n_dot_wo, sample, layer);

	result.pdf = n_dot_wi * math::Pi_inv;
	result.reflection = on * layer.diffuse_color;
	result.wi = wi;
	result.type.clear_set(bxdf::Type::Diffuse_reflection);

	return n_dot_wi;
}

template<typename Layer>
float Isotropic::f(float3_p wi, float n_dot_wi, float n_dot_wo,
				   const Sample& sample, const Layer& layer) {
	float wi_dot_wo = math::dot(wi, sample.wo_);

	float s = wi_dot_wo - n_dot_wi * n_dot_wo;

	float t;
	if (s >= 0.f) {
		t = std::min(1.f, n_dot_wi / n_dot_wo);
	} else {
		t = n_dot_wi;
	}

	float a2 = layer.a2;
	float a = 1.f - 0.5f * (a2 / (a2 + 0.33f));
	float b = 0.45f * (a2 / (a2 + 0.09f));

	return math::Pi_inv * (a + b * s * t);
}

}}}

