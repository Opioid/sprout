#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace oren_nayar {

class Oren_nayar {
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

}}}

