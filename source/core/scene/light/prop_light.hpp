#pragma once

#include "light.hpp"

namespace scene {

namespace shape { class Node_stack; }

namespace light {

class Prop_light : public Light {

public:

	void init(Prop* prop, uint32_t part);

	virtual const Transformation& transformation_at(
			float time, Transformation& transformation) const final override;

	virtual void sample(const Transformation& transformation, float time,
						float3_p p, float3_p n, bool total_sphere,
						sampler::Sampler& sampler, uint32_t sampler_dimension, Worker& worker,
						Sampler_filter filter, Sample& result) const override;

	virtual float pdf(const Transformation& transformation,
					  float3_p p, float3_p wi, bool total_sphere,
					  Worker& worker, Sampler_filter filter) const override;

	virtual float3 power(const math::aabb& scene_bb) const final override;

	virtual void prepare_sampling(uint32_t light_id, thread::Pool& pool) override;

	virtual bool equals(const Prop* prop, uint32_t part) const final override;

protected:

	Prop* prop_;

	uint32_t part_;
};

}}
