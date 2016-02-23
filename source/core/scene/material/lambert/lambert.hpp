#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace lambert {

template<typename Sample>
class Lambert {
public:

	math::vec3 evaluate(const Sample& sample, const math::vec3& wi, float n_dot_wi) const;

	float pdf(const Sample& sample, const math::vec3& wi, float n_dot_wi) const;

	float importance_sample(const Sample& sample, sampler::Sampler& sampler, bxdf::Result& result) const;
};


}}}
