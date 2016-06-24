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
	static float3 evaluate(float3_p wi, float n_dot_wi, float n_dot_wo,
						   const Sample& sample, const Layer& layer, float& pdf);

	template<typename Layer>
	static float importance_sample(float n_dot_wo, const Sample& sample, const Layer& layer,
								   sampler::Sampler& sampler, bxdf::Result& result);

private:

	template<typename Layer>
	static float f(float3_p wi, float n_dot_wi, float n_dot_wo,
				   const Sample& sample, const Layer& layer);
};

}}}

