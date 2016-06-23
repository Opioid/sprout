#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace lambert {

class Isotropic {

public:

	template<typename Sample>
	static float3 evaluate(float3_p wi, float n_dot_wi,
								 const Sample& sample, float& pdf);

	template<typename Sample>
	static float pdf(float3_p wi, float n_dot_wi, const Sample& sample);

	template<typename Sample>
	static float importance_sample(const Sample& sample, sampler::Sampler& sampler,
								   bxdf::Result& result);
};


}}}
