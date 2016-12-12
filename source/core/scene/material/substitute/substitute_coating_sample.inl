#pragma once

#include "substitute_coating_sample.hpp"
#include "substitute_base_sample.inl"
#include "scene/material/material_sample.inl"
#include "base/math/vector.inl"
#include "base/math/math.hpp"

namespace scene { namespace material { namespace substitute {

template<typename Coating_layer>
float3 Sample_coating<Coating_layer>::evaluate(float3_p wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return math::float3_identity;
	}

	return base_evaluate_and_coating(wi, coating_, pdf);
}

template<typename Coating_layer>
void Sample_coating<Coating_layer>::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	base_sample_and_coating(coating_, sampler, result);
}

}}}
