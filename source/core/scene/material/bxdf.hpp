#pragma once

namespace scene { namespace material {

template<typename Sample>
class BXDF {
public:

	BXDF(const Sample& sample) : sample_(sample) {}
	virtual ~BXDF() {}

	virtual math::float3 evaluate(const math::float3& wi) const = 0;
	virtual math::float3 importance_sample(sampler::Sampler& sampler, math::float3& wi, float& pdf) const = 0;

protected:

	const Sample& sample_;
};

}}
