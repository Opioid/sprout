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
	static float3 reflection(const float3& wi, const float3& wo, float n_dot_wi, float n_dot_wo,
							 const Layer& layer, float& pdf);

	template<typename Layer>
	static float reflect(const float3& wo, float n_dot_wo, const Layer& layer,
						 sampler::Sampler& sampler, bxdf::Sample& result);

private:

	static float f(const float3& wi, const float3& wo, float n_dot_wi, float n_dot_wo, float alpha2);
};

}}}

