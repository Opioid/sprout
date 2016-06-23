#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace oren_nayar {

class Isotropic {

public:

	template<typename Sample>
	static float3 evaluate(float3_p wi, float n_dot_wi, float n_dot_wo,
								 const Sample& sample, float& pdf);

	template<typename Sample>
	static float importance_sample(float n_dot_wo, const Sample& sample,
								   sampler::Sampler& sampler, bxdf::Result& result);

private:

	template<typename Sample>
	static float f(float3_p wi, float n_dot_wi, float n_dot_wo, const Sample& sample);
};

}}}

