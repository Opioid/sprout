#pragma once

namespace scene { namespace material {

class BXDF {
public:

	virtual math::float3 evaluate(const math::float3& wi) const = 0;
	virtual math::float3 importance_sample(sampler::Sampler& sampler, math::float3& wi, float& pdf) const = 0;
};

}}
