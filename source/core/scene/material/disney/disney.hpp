#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

class Sample;

namespace disney {

class Isotropic {

public:

	template<typename Layer>
	static float3 evaluate(float3_p wi, float3_p wo, float n_dot_wi,
						   float n_dot_wo, const Layer& layer, float& pdf);

	template<typename Layer>
	static float sample(float3_p wo, float n_dot_wo, const Layer& layer,
						sampler::Sampler& sampler, bxdf::Result& result);

private:

	template<typename Layer>
	static float3 evaluate(float3_p wi, float3_p h, float n_dot_wi,
						   float n_dot_wo, const Layer& layer);

	static float f_D90(float3_p wi, float3_p h, float roughness);
};

}}}

