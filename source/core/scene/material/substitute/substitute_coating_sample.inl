#pragma once

#include "substitute_coating_sample.hpp"
#include "substitute_base_sample.inl"
#include "scene/material/material_sample.inl"
#include "base/math/vector3.inl"
#include "base/math/math.hpp"

namespace scene::material::substitute {

template<typename Coating_layer>
bxdf::Result Sample_coating<Coating_layer>::evaluate(const float3& wi) const {
	if (!same_hemisphere(wo_)) {
		return { float3::identity(), 0.f };
	}

	return base_and_coating_evaluate(wi, coating_);
}

template<typename Coating_layer>
void Sample_coating<Coating_layer>::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	base_and_coating_sample(coating_, sampler, result);
}

}
