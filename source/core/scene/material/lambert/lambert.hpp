#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

class Sample;

namespace lambert {

class Isotropic {

public:

	template<typename Layer>
	static float3 evaluate(float3_p wi, float n_dot_wi,
						   const Layer& layer, float& pdf);

	template<typename Layer>
	static float pdf(float3_p wi, float n_dot_wi, const Layer& layer);

	template<typename Layer>
	static float importance_sample(const Layer& layer, sampler::Sampler& sampler,
								   bxdf::Result& result);
};


}}}
