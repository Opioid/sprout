#pragma once

#include "base/math/vector.hpp"
#include "base/math/bounding/aabb.hpp"
#include <vector>

namespace sampler {

class Sampler;

}

namespace scene { namespace light {

struct Sample;

class Light {
public:

	virtual ~Light() {}

	virtual void sample(const math::float3& p, float time, uint32_t max_samples, sampler::Sampler& sampler,
						std::vector<Sample>& samples) const = 0;

	virtual math::float3 energy(const math::aabb& scene_bb) const = 0;

	virtual void prepare_sampling() = 0;
};

}}
