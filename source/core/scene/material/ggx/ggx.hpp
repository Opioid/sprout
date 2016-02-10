#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace ggx {

template<typename Sample>
class Schlick_isotropic {
public:

	math::float3 evaluate(const Sample& sample,
						  const math::float3& wi, float n_dot_wi, float n_dot_wo,
						  float& pdf) const;

	float importance_sample(const Sample& sample,
							sampler::Sampler& sampler, float n_dot_wo,
							bxdf::Result& result) const;
};

template<typename Sample>
class Conductor_isotropic {
public:

	math::float3 evaluate(const Sample& sample,
						  const math::float3& wi, float n_dot_wi, float n_dot_wo,
						  float& pdf) const;

	float importance_sample(const Sample& sample,
							sampler::Sampler& sampler, float n_dot_wo,
							bxdf::Result& result) const;
};

template<typename Sample>
class Conductor_anisotropic {
public:

	math::float3 evaluate(const Sample& sample,
						  const math::float3& wi, float n_dot_wi, float n_dot_wo,
						  float& pdf) const;

	float importance_sample(const Sample& sample,
							sampler::Sampler& sampler, float n_dot_wo,
							bxdf::Result& result) const;
};

float distribution_isotropic(float n_dot_h, float a2);

float distribution_anisotropic(float n_dot_h, float x_dot_h, float y_dot_h, math::float2 a2, float axy);

float geometric_shadowing(float n_dot_wi, float n_dot_wo, float a2);

float clamp_a2(float a2);

constexpr float min_a2 = 0.00000003f;

}}}
