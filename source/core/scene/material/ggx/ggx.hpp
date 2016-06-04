#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace ggx {

class Schlick_isotropic {

public:

	template<typename Sample>
	float init_importance_sample(const Sample& sample, float a2,
								 sampler::Sampler& sampler, float n_dot_wo,
								 bxdf::Result& result);

	template<typename Sample>
	void init_evaluate(const Sample& sample, math::pfloat3 wi);

	float evaluate(float f0, float a2, float n_dot_wi, float n_dot_wo,
				   float& fresnel, float& pdf) const;

	math::float3 evaluate(math::pfloat3 f0, float a2, float n_dot_wi, float n_dot_wo,
						  float& pdf) const;

	template<typename Sample>
	static math::float3 evaluate(const Sample& sample,
								 math::pfloat3 wi, float n_dot_wi, float n_dot_wo,
								 float& pdf);

	template<typename Sample>
	static float importance_sample(const Sample& sample,
								   sampler::Sampler& sampler, float n_dot_wo,
								   bxdf::Result& result);
private:

	float n_dot_h_;
	float wo_dot_h_;
};

class Conductor_isotropic {

public:

	template<typename Sample>
	static math::float3 evaluate(const Sample& sample,
								 math::pfloat3 wi, float n_dot_wi, float n_dot_wo,
								 float& pdf);

	template<typename Sample>
	static float importance_sample(const Sample& sample,
								   sampler::Sampler& sampler, float n_dot_wo,
								   bxdf::Result& result);
};

class Conductor_anisotropic {

public:

	template<typename Sample>
	static math::float3 evaluate(const Sample& sample,
								 math::pfloat3 wi, float n_dot_wi, float n_dot_wo,
								 float& pdf);

	template<typename Sample>
	static float importance_sample(const Sample& sample,
								   sampler::Sampler& sampler, float n_dot_wo,
								   bxdf::Result& result);
};

float distribution_isotropic(float n_dot_h, float a2);

float distribution_anisotropic(float n_dot_h, float x_dot_h, float y_dot_h,
							   math::float2 a2, float axy);

float geometric_shadowing(float n_dot_wi, float n_dot_wo, float a2);

float clamp_a2(float a2);

constexpr float Min_a2 = 0.0000000299f;

}}}
