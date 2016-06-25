#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

class Sample;

namespace oren_nayar {

class Isotropic {

public:

	template<typename Layer>
	static float3 evaluate(float3_p wi, float3_p wo, float n_dot_wi, float n_dot_wo,
						   const Layer& layer, float& pdf);

	template<typename Layer>
	static float importance_sample(float3_p wo, float n_dot_wo, const Layer& layer,
								   sampler::Sampler& sampler, bxdf::Result& result);

private:

	static float f(float3_p wi, float3_p wo, float n_dot_wi, float n_dot_wo, float a2);
};

}}}

