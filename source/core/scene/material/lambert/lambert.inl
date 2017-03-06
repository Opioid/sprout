#pragma once

#include "lambert.hpp"
#include "scene/material/bxdf.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/math.hpp"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

namespace scene { namespace material { namespace lambert {

template<typename Layer>
float3 Isotropic::reflection(float3_p color, float n_dot_wi, const Layer& layer, float& pdf) {
	pdf = n_dot_wi * math::Pi_inv;
	float3 result = math::Pi_inv * color;

	SOFT_ASSERT(testing::check(result, float3::identity(), float3::identity(), pdf, layer));

	return result;
}

template<typename Layer>
float Isotropic::reflect(float3_p color, const Layer& layer,
						 sampler::Sampler& sampler, bxdf::Result& result) {
	float2 s2d = sampler.generate_sample_2D();

	float3 is = math::sample_hemisphere_cosine(s2d);

	float3 wi = math::normalized(layer.tangent_to_world(is));

	float n_dot_wi = layer.clamped_n_dot(wi);

	result.pdf = n_dot_wi * math::Pi_inv;
	result.reflection = math::Pi_inv * color;
	result.wi = wi;
	result.type.clear_set(bxdf::Type::Diffuse_reflection);

	SOFT_ASSERT(testing::check(result, float3::identity(), layer));

	return n_dot_wi;
}

}}}
