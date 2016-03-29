#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace lambert {

class Lambert {
public:

	template<typename Sample>
	static math::float3 evaluate(const Sample& sample, const math::float3& wi, float n_dot_wi);

	template<typename Sample>
	static float pdf(const Sample& sample, const math::float3& wi, float n_dot_wi);

	template<typename Sample>
	static float importance_sample(const Sample& sample, sampler::Sampler& sampler, bxdf::Result& result);
};


}}}
