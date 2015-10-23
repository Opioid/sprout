#pragma once

#include "base/math/vector.hpp"
#include "base/math/bounding/aabb.hpp"

namespace image { namespace texture { namespace sampler {

class Sampler_2D;

}}}

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
						const math::float3& p, const math::float3& n, bool total_sphere,
						const image::texture::sampler::Sampler_2D& image_sampler, sampler::Sampler& sampler,
						Sample& result) const = 0;

	void sample(float time, const math::float3& p, const math::float3& n, bool total_sphere,
				const image::texture::sampler::Sampler_2D& image_sampler, sampler::Sampler& sampler,
				Sample& result) const;

	virtual math::float3 evaluate(const math::float3& wi) const = 0;

	virtual float pdf(const entity::Composed_transformation& transformation,
					  const math::float3& p, const math::float3& wi, bool total_sphere,
					  const image::texture::sampler::Sampler_2D& image_sampler) const = 0;

	virtual math::float3 power(const math::aabb& scene_bb) const = 0;

	virtual void prepare_sampling() = 0;

	virtual bool equals(const Prop* prop, uint32_t part) const = 0;
};

}}
