#pragma once

#include "scene/entity/entity.hpp"
#include <vector>

namespace rendering { namespace sampler {

class Sampler;

}}

namespace scene { namespace light {

struct Sample;

class Light : public Entity {
public:

	virtual void sample(const math::float3& p, float time, uint32_t max_samples, rendering::sampler::Sampler& sampler,
						std::vector<Sample>& samples) const = 0;
};

}}
