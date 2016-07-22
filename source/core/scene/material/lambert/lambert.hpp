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
	static float3 evaluate(float3_p color, float n_dot_wi,
						   const Layer& layer, float& pdf);

	template<typename Layer>
	static float sample(float3_p color, const Layer& layer,
								   sampler::Sampler& sampler, bxdf::Result& result);
};


}}}
