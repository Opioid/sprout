#pragma once

#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace lambert {

template<typename Sample>
class Lambert {
public:

	math::float3 evaluate(const Sample& sample,
						  const math::float3& wi, float n_dot_wi) const;

	float pdf(const Sample& sample,
			  const math::float3& wi, float n_dot_wi) const;

	float importance_sample(const Sample& sample,
							sampler::Sampler& sampler,
							BxDF_result& result) const;
};


}}}
