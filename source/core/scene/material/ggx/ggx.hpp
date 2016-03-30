#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace ggx {

class Schlick_isotropic {
public:

	template<typename Sample>
	static math::float3 evaluate(const Sample& sample,
								 const math::float3& wi, float n_dot_wi, float n_dot_wo,
								 float& pdf);

	template<typename Sample>
	static math::float3 evaluate_and_clearcoat(const Sample& sample, float clearcoat_f0,
											   const math::float3& wi, float n_dot_wi, float n_dot_wo,
											   float& pdf, float& clearcoat);

	template<typename Sample>
	static float importance_sample(const Sample& sample,
								   sampler::Sampler& sampler, float n_dot_wo,
								   bxdf::Result& result);

	template<typename Sample>
	static float importance_sample(const Sample& sample, float f0, float a2,
								   sampler::Sampler& sampler, float n_dot_wo,
								   bxdf::Result& result);

	template<typename Sample>
	static float importance_sample_and_clearcoat(const Sample& sample, float clearcoat_f0,
												 sampler::Sampler& sampler, float n_dot_wo,
												 bxdf::Result& result, float& clearcoat);
};

class Conductor_isotropic {
public:

	template<typename Sample>
	static math::float3 evaluate(const Sample& sample,
								 const math::float3& wi, float n_dot_wi, float n_dot_wo,
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
								 const math::float3& wi, float n_dot_wi, float n_dot_wo,
								 float& pdf);

	template<typename Sample>
	static float importance_sample(const Sample& sample,
								   sampler::Sampler& sampler, float n_dot_wo,
								   bxdf::Result& result);
};

float distribution_isotropic(float n_dot_h, float a2);

float distribution_anisotropic(float n_dot_h, float x_dot_h, float y_dot_h, math::float2 a2, float axy);

float geometric_shadowing(float n_dot_wi, float n_dot_wo, float a2);

float clamp_a2(float a2);

constexpr float min_a2 = 0.00000003f;

}}}
