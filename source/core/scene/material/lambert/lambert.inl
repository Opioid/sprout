#pragma once

#include "lambert.hpp"
#include "scene/material/bxdf.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/math.hpp"

namespace scene { namespace material { namespace lambert {

template<typename Sample>
math::float3 Lambert::evaluate(const Sample& sample, const math::float3& /*wi*/, float /*n_dot_wi*/) {
	return math::Pi_inv * sample.diffuse_color_;
}

template<typename Sample>
float Lambert::pdf(const Sample& /*sample*/, const math::float3& /*wi*/, float n_dot_wi) {
	return n_dot_wi * math::Pi_inv;
}

template<typename Sample>
float Lambert::importance_sample(const Sample& sample, sampler::Sampler& sampler, bxdf::Result& result) {
	math::float2 s2d = sampler.generate_sample_2D();

	math::float3 is = math::sample_hemisphere_cosine(s2d);
	result.wi = math::normalized(sample.tangent_to_world(is));

	float n_dot_wi = std::max(math::dot(sample.n_, result.wi), 0.00001f);
	result.pdf = n_dot_wi * math::Pi_inv;
	result.reflection = math::Pi_inv * sample.diffuse_color_;

	result.type.clear_set(bxdf::Type::Diffuse_reflection);

	return n_dot_wi;
}

}}}
