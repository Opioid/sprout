#pragma once

#include "scene/material/sampler_settings.hpp"
#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"

namespace image { namespace texture { namespace sampler { class Sampler_2D; }}}

namespace sampler { class Sampler; }

namespace thread { class Pool; }

namespace scene {

namespace entity { struct Composed_transformation; }
namespace shape { struct Intersection; class Node_stack; }

class Worker;
class Prop;
struct Ray;

namespace light {

struct Sample;

class Light {

public:

	using Transformation = entity::Composed_transformation;
	using Sampler_filter = material::Sampler_settings::Filter;
	using Intersection   = shape::Intersection;

	virtual ~Light() {}

	virtual const Transformation& transformation_at(
			float time, Transformation& transformation) const = 0;

	virtual void sample(const Transformation& transformation,
						const float3& p, const float3& n, float time, bool total_sphere,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Worker& worker, Sampler_filter filter, Sample& result) const = 0;

	void sample(const float3& p, const float3& n, float time, bool total_sphere,
				sampler::Sampler& sampler, uint32_t sampler_dimension, Worker& worker,
				Sampler_filter filter, Sample& result) const;

	void sample(const float3& p, float time,
				sampler::Sampler& sampler, uint32_t sampler_dimension,
				Worker& worker, Sampler_filter filter, Sample& result) const;

	virtual float pdf(const Ray& ray, const Intersection& intersection, bool total_sphere,
					  Worker& worker, Sampler_filter filter) const = 0;

	virtual float3 power(const math::AABB& scene_bb) const = 0;

	virtual void prepare_sampling(uint32_t light_id, thread::Pool& pool) = 0;

	virtual bool equals(const Prop* prop, uint32_t part) const = 0;\

	static bool is_light(uint32_t id);
};

}}
