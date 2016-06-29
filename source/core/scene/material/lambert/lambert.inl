#pragma once

#include "lambert.hpp"
#include "scene/material/bxdf.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/math.hpp"

namespace scene { namespace material { namespace lambert {

template<typename Layer>
float3 Isotropic::evaluate(float3_p /*wi*/, float n_dot_wi,
						   const Layer& layer, float& pdf) {
	pdf = n_dot_wi * math::Pi_inv;
	return math::Pi_inv * layer.diffuse_color;
}

template<typename Layer>
float Isotropic::pdf(float3_p /*wi*/, float n_dot_wi, const Layer& /*layer*/) {
	return n_dot_wi * math::Pi_inv;
}

template<typename Layer>
float Isotropic::importance_sample(const Layer& layer, sampler::Sampler& sampler,
								   bxdf::Result& result) {
	float2 s2d = sampler.generate_sample_2D();

	float3 is = math::sample_hemisphere_cosine(s2d);

	float3 wi = math::normalized(layer.tangent_to_world(is));

	float n_dot_wi = layer.clamped_n_dot(wi);

	result.pdf = n_dot_wi * math::Pi_inv;
	result.reflection = math::Pi_inv * layer.diffuse_color;
	result.wi = wi;
	result.type.clear_set(bxdf::Type::Diffuse_reflection);

	return n_dot_wi;
}

}}}
