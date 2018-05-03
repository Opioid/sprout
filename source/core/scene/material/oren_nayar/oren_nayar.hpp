#pragma once

#include "base/math/vector3.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

class Sample;

namespace oren_nayar {

class Isotropic {

public:

	template<typename Layer>
	static float3 reflection(f_float3 wi, f_float3 wo, float n_dot_wi, float n_dot_wo,
							 Layer const& layer, float& pdf);

	template<typename Layer>
	static float reflect(f_float3 wo, float n_dot_wo, Layer const& layer,
						 sampler::Sampler& sampler, bxdf::Sample& result);

private:

	static float f(f_float3 wi, f_float3 wo, float n_dot_wi, float n_dot_wo, float alpha2);
};

}}}

