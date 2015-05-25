#pragma once

#include "base/math/vector.hpp"
#include "base/math/bounding/aabb.hpp"
#include <vector>

namespace sampler {

class Sampler;

}

namespace scene {

class Prop;
struct Composed_transformation;

namespace light {

struct Sample;

class Light {
public:

	virtual ~Light() {}

	virtual void transformation_at(float time, Composed_transformation& transformation) const = 0;

	virtual void sample(const math::float3& p, const Composed_transformation& transformation, uint32_t max_samples, sampler::Sampler& sampler,
						std::vector<Sample>& samples) const = 0;

	void sample(const math::float3& p, float time, uint32_t max_samples, sampler::Sampler& sampler, std::vector<Sample>& samples) const;

	virtual float pdf(const math::float3& p, const math::float3& wi, const Composed_transformation& transformation) const = 0;

	virtual math::float3 energy(const math::aabb& scene_bb) const = 0;

	virtual void prepare_sampling() = 0;

	virtual bool equals(const Prop* prop, uint32_t part) const = 0;
};

}}
