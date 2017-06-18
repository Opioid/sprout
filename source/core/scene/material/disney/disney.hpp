#pragma once

#include "base/math/vector3.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

class Sample;

namespace disney {

class Isotropic {

public:

	template<typename Layer>
	static float3 reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
							 const Layer& layer, float& pdf);

	template<typename Layer>
	static float reflect(const float3& wo, float n_dot_wo, const Layer& layer,
						 sampler::Sampler& sampler, bxdf::Result& result);

private:

	template<typename Layer>
	static float3 evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, const Layer& layer);
};

class Isotropic_no_lambert {

public:

	template<typename Layer>
	static float3 reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
							 const Layer& layer, float& pdf);

	template<typename Layer>
	static float reflect(const float3& wo, float n_dot_wo, const Layer& layer,
						 sampler::Sampler& sampler, bxdf::Result& result);

private:

	template<typename Layer>
	static float3 evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, const Layer& layer);
};

}}}

