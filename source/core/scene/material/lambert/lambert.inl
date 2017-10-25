#pragma once

#include "lambert.hpp"
#include "scene/material/bxdf.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling/sampling.hpp"
#include "base/math/vector3.inl"
#include "base/math/math.hpp"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

namespace scene { namespace material { namespace lambert {

template<typename Layer>
float3 Isotropic::reflection(const float3& color, float n_dot_wi, const Layer& layer, float& pdf) {
	pdf = n_dot_wi * math::Pi_inv;
	const float3 result = math::Pi_inv * color;

	(void)layer;
	SOFT_ASSERT(testing::check(result, float3::identity(), float3::identity(), pdf, layer));

	return result;
}

template<typename Layer>
float Isotropic::reflect(const float3& color, const Layer& layer,
						 sampler::Sampler& sampler, bxdf::Result& result) {
	const float2 s2d = sampler.generate_sample_2D();

	const float3 is = math::sample_hemisphere_cosine(s2d);

	const float3 wi = math::normalize(layer.tangent_to_world(is));

	const float n_dot_wi = layer.clamp_n_dot(wi);

	result.pdf = n_dot_wi * math::Pi_inv;
	result.reflection = math::Pi_inv * color;
	result.wi = wi;
	result.type.clear(bxdf::Type::Diffuse_reflection);

	SOFT_ASSERT(testing::check(result, float3::identity(), layer));

	return n_dot_wi;
}

}}}
