#pragma once

#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace oren_nayar {

template<typename Sample>
class Oren_nayar {
public:

	math::float3 evaluate(const Sample& sample,
						  const math::float3& wi, float n_dot_wi, float n_dot_wo,
						  float& pdf) const;

	float importance_sample(const Sample& sample,
							sampler::Sampler& sampler, float n_dot_wo,
							BxDF_result& result) const;
};

}}}

