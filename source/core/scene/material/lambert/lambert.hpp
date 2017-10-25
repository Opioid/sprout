#pragma once

#include "base/math/vector3.hpp"

namespace sampler { class Sampler; }

namespace scene::material {

namespace bxdf { struct Result; struct Sample; }

class Sample;

namespace lambert {

class Isotropic {

public:

	template<typename Layer>
	static bxdf::Result reflection(const float3& color, float n_dot_wi,
								   const Layer& layer);

	template<typename Layer>
	static float reflect(const float3& color, const Layer& layer,
						 sampler::Sampler& sampler, bxdf::Sample& result);
};

}}
