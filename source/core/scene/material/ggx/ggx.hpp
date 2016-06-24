#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

class Sample;

namespace bxdf { struct Result; }

namespace ggx {

class Isotropic {

public:

	template<typename Layer>
	float init_importance_sample(float n_dot_wo, float a2,
								 const Sample& sample, const Layer& layer,
								 sampler::Sampler& sampler, bxdf::Result& result);

	template<typename Layer>
	void init_evaluate(float3_p wi, const Sample& sample, const Layer& layer);

	// This method is intended for cases where the fresnel value
	// will also be used for blending with other layers
	template<typename Fresnel>
	float3 evaluate(float n_dot_wi, float n_dot_wo, float a2, const Fresnel& fresnel,
						  float3& fresnel_result, float& pdf) const;

	template<typename Fresnel>
	float3 evaluate(float n_dot_wi, float n_dot_wo, float a2,
					const Fresnel& fresnel, float& pdf) const;

	template<typename Layer, typename Fresnel>
	static float3 evaluate(float3_p wi, float n_dot_wi, float n_dot_wo,
						   const Sample& sample, const Layer& layer,
						   const Fresnel& fresnel, float& pdf);

	template<typename Layer, typename Fresnel>
	static float importance_sample(float n_dot_wo, const Sample& sample, const Layer& layer,
								   const Fresnel& fresnel, sampler::Sampler& sampler,
								   bxdf::Result& result);
private:

	float n_dot_h_;
	float wo_dot_h_;
};

class Anisotropic {

public:

	template<typename Layer, typename Fresnel>
	static float3 evaluate(float3_p wi, float n_dot_wi, float n_dot_wo,
						   const Sample& sample, const Layer& layer,
						   const Fresnel& fresnel, float& pdf);

	template<typename Layer, typename Fresnel>
	static float importance_sample(float n_dot_wo, const Sample& sample,
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
