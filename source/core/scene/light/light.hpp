#pragma once

#include "base/math/vector.hpp"
#include "base/math/bounding/aabb.hpp"
#include <vector>

namespace image { namespace sampler {

class Sampler_2D;

}}

namespace sampler {

class Sampler;

}

namespace scene {

namespace entity {

struct Composed_transformation;

}

class Prop;

namespace light {

struct Sample;

class Light {
public:

	virtual ~Light() {}

	virtual void transformation_at(float time, entity::Composed_transformation& transformation) const = 0;

	virtual void sample(const entity::Composed_transformation& transformation,
						const math::float3& p, const math::float3& n,
						const image::sampler::Sampler_2D& image_sampler, sampler::Sampler& sampler,
						uint32_t max_samples, std::vector<Sample>& samples) const = 0;

	void sample(float time, const math::float3& p, const math::float3& n,
				const image::sampler::Sampler_2D& image_sampler, sampler::Sampler& sampler,
				uint32_t max_samples, std::vector<Sample>& samples) const;

	virtual math::float3 evaluate(const math::float3& wi) const = 0;

	virtual float pdf(const entity::Composed_transformation& transformation,
					  const math::float3& p, const math::float3& wi) const = 0;

	virtual math::float3 power(const math::aabb& scene_bb) const = 0;

	virtual void prepare_sampling() = 0;

	virtual bool equals(const Prop* prop, uint32_t part) const = 0;
};

}}
