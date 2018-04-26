#pragma once

#include "base/math/vector3.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; struct Sample; }

class Sample;

namespace disney {

class Isotropic {

public:

	template<typename Layer>
	static bxdf::Result reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
								   const Layer& layer);

	template<typename Layer>
	static float reflect(f_float3 wo, float n_dot_wo, const Layer& layer,
						 sampler::Sampler& sampler, bxdf::Sample& result);

private:

	template<typename Layer>
	static float3 evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, const Layer& layer);
};

class Isotropic_no_lambert {

public:

	template<typename Layer>
	static bxdf::Result reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
								   const Layer& layer);

	template<typename Layer>
	static float reflect(f_float3 wo, float n_dot_wo, const Layer& layer,
						 sampler::Sampler& sampler, bxdf::Sample& result);

private:

	template<typename Layer>
	static float3 evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, const Layer& layer);
};

class Isotropic_scaled_lambert {

public:

	struct Data {
		float lambert_scale;
	};

	template<typename Layer>
	static bxdf::Result reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
								   const Layer& layer);

	template<typename Layer>
	static float reflect(f_float3 wo, float n_dot_wo, const Layer& layer,
						 sampler::Sampler& sampler, bxdf::Sample& result);

private:

	template<typename Layer>
	static float3 evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, const Layer& layer);
};

}}}

