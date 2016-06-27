#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

class Sample;

namespace bxdf { struct Result; }

namespace ggx {

class Isotropic {

public:

	template<typename Layer, typename Fresnel>
	static float3 evaluate(float3_p wi, float3_p wo, float n_dot_wi, float n_dot_wo,
						   const Layer& layer, const Fresnel& fresnel, float& pdf);

	template<typename Layer, typename Fresnel>
	static float importance_sample(float3_p wo, float n_dot_wo, const Layer& layer,
								   const Fresnel& fresnel, sampler::Sampler& sampler,
								   bxdf::Result& result);

	template<typename Layer, typename Fresnel>
	static float3 evaluate(float3_p wi, float3_p wo, float n_dot_wi, float n_dot_wo,
						   const Layer& layer, const Fresnel& fresnel,
						   float3& fresnel_result, float& pdf);

	template<typename Layer, typename Fresnel>
	static float importance_sample(float3_p wo, float n_dot_wo, const Layer& layer,
								   const Fresnel& fresnel, sampler::Sampler& sampler,
								   float3& fresnel_result, bxdf::Result& result);
};

class Anisotropic {

public:

	template<typename Layer, typename Fresnel>
	static float3 evaluate(float3_p wi, float3_p wo, float n_dot_wi, float n_dot_wo,
						   const Layer& layer, const Fresnel& fresnel, float& pdf);

	template<typename Layer, typename Fresnel>
	static float importance_sample(float3_p wo, float n_dot_wo,
								   const Layer& layer, const Fresnel& fresnel,
								   sampler::Sampler& sampler, bxdf::Result& result);
};

float distribution_isotropic(float n_dot_h, float a2);

float distribution_anisotropic(float n_dot_h, float x_dot_h, float y_dot_h,
							   float2 a2, float axy);

float geometric_visibility(float n_dot_wi, float n_dot_wo, float a2);

float clamp_a2(float a2);

constexpr float Min_a2 = 0.0000000299f;

}}}
