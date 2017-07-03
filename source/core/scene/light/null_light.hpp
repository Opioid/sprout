#pragma once

#include "light.hpp"

namespace scene {

namespace shape { class Node_stack; }

namespace light {

class Null_light : public Light {

public:

	virtual const Transformation& transformation_at(
			float time, Transformation& transformation) const override final;

	virtual void sample(const Transformation& transformation,
						const float3& p, const float3& n, float time, bool total_sphere,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Worker& worker, Sampler_filter filter, Sample& result) const override;

	virtual float pdf(const Transformation& transformation,
					  const float3& p, const float3& wi, bool total_sphere,
					  Worker& worker, Sampler_filter filter) const override;

	virtual float3 power(const math::AABB& scene_bb) const override final;

	virtual void prepare_sampling(uint32_t light_id, thread::Pool& pool) override;

	virtual bool equals(const Prop* prop, uint32_t part) const override final;
};

}}
