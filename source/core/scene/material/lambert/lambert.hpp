#pragma once

#include "base/math/vector3.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

class Sample;

namespace lambert {

class Isotropic {

public:

	template<typename Layer>
	static float3 reflection(const float3& color, float n_dot_wi,
							 const Layer& layer, float& pdf);

	template<typename Layer>
	static float reflect(const float3& color, const Layer& layer,
						 sampler::Sampler& sampler, bxdf::Result& result);
};

}}}
