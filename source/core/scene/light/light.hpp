#pragma once

#include "scene/material/sampler_settings.hpp"
#include "base/math/vector.hpp"
#include "base/math/bounding/aabb.hpp"

namespace image { namespace texture { namespace sampler { class Sampler_2D; }}}

namespace sampler { class Sampler; }

namespace scene {

namespace entity { struct Composed_transformation; }
namespace shape { class Node_stack; }

class Worker;
class Prop;

namespace light {

struct Sample;

class Light {

public:

	using Entity_transformation = entity::Composed_transformation;
	using Sampler_filter = material::Sampler_settings::Filter;

	virtual ~Light() {}

	virtual const Entity_transformation& transformation_at(
			float time, Entity_transformation& transformation) const = 0;

	virtual void sample(const Entity_transformation& transformation, float time,
						float3_p p, float3_p n, bool total_sphere,
						sampler::Sampler& sampler, Worker& worker,
						Sampler_filter filter, Sample& result) const = 0;

	void sample(float time, float3_p p, float3_p n,
				bool total_sphere, sampler::Sampler& sampler, Worker& worker,
				Sampler_filter filter, Sample& result) const;

	void sample(float time, float3_p p, sampler::Sampler& sampler,
				Worker& worker, Sampler_filter filter, Sample& result) const;

	virtual float pdf(const Entity_transformation& transformation,
					  float3_p p, float3_p wi, bool total_sphere,
					  Worker& worker, Sampler_filter filter) const = 0;

	virtual float3 power(const math::aabb& scene_bb) const = 0;

	virtual void prepare_sampling() = 0;

	virtual bool equals(const Prop* prop, uint32_t part) const = 0;
};

}}
