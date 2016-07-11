#pragma once

#include "disney.hpp"
#include "scene/material/bxdf.hpp"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sampling.inl"
\
namespace scene { namespace material { namespace disney {

template<typename Layer>
float3 Isotropic::evaluate(float3_p wi, float3_p wo, float n_dot_wi, float n_dot_wo,
						   const Layer& layer, float& pdf) {
	float3 h = math::normalized(wo + wi);

	float fmo = f_D90(wi, h, layer.roughness) - 1.f;

	float a = 1.f + fmo * std::pow(1.f - n_dot_wi, 5.f);
	float b = 1.f + fmo * std::pow(1.f - n_dot_wo, 5.f);

	pdf = n_dot_wi * math::Pi_inv;
	return a * b * (math::Pi_inv * layer.diffuse_color);
}

template<typename Layer>
float Isotropic::importance_sample(float3_p wo, float n_dot_wo, const Layer& layer,
								   sampler::Sampler& sampler, bxdf::Result& result) {
	float2 s2d = sampler.generate_sample_2D();

	float3 is = math::sample_hemisphere_cosine(s2d);
	float3 wi = math::normalized(layer.tangent_to_world(is));

	float n_dot_wi = layer.clamped_n_dot(wi);

	float on = f(wi, wo, n_dot_wi, n_dot_wo, layer.a2);

	result.pdf = n_dot_wi * math::Pi_inv;
	result.reflection = on * layer.diffuse_color;
	result.wi = wi;
	result.type.clear_set(bxdf::Type::Diffuse_reflection);

	return n_dot_wi;
}

inline float Isotropic::f_D90(float3_p wi, float3_p h, float roughness) {
	float wi_dot_h = math::dot(wi, h);
	return 0.5f + 2.f * roughness * (wi_dot_h * wi_dot_h);
}

}}}

