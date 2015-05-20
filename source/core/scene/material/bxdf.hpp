#pragma once

#include "base/math/vector.hpp"
#include "base/flags/flags.hpp"

namespace sampler {

class Sampler;

}

namespace scene { namespace material {

enum class BxDF_type {
	Reflection		= 1 << 0,
	Transmission	= 1 << 1
};

struct BxDF_result {
	math::float3 reflection;
	math::float3 wi;
	float        pdf;

	typedef flags::Flags<BxDF_type> Type;
	Type type;
};

template<typename Sample>
class BxDF {
public:

	BxDF(const Sample& sample) : sample_(sample) {}
	virtual ~BxDF() {}

	virtual math::float3 evaluate(const math::float3& wi) const = 0;
	virtual void importance_sample(sampler::Sampler& sampler, BxDF_result& result) const = 0;

protected:

	const Sample& sample_;
};

}}
