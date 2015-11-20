#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace ggx {

template<typename Sample>
class GGX_Schlick {
public:

	math::float3 evaluate(const Sample& sample,
						  const math::float3& wi, float n_dot_wi, float n_dot_wo,
						  float& pdf) const;

	float importance_sample(const Sample& sample,
							sampler::Sampler& sampler, float n_dot_wo,
							bxdf::Result& result) const;
};

template<typename Sample>
class GGX_Conductor {
public:

	math::float3 evaluate(const Sample& sample,
						  const math::float3& wi, float n_dot_wi, float n_dot_wo,
						  float& pdf) const;

	float importance_sample(const Sample& sample,
							sampler::Sampler& sampler, float n_dot_wo,
							bxdf::Result& result) const;
};

math::float3 f(float wo_dot_h, const math::float3& f0);
float f(float wo_dot_h, float f0);

float d(float n_dot_h, float a2);

float g(float n_dot_wi, float n_dot_wo, float a2);

}}}
