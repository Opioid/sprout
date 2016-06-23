#pragma once

#include "light.hpp"

namespace scene {

namespace shape { class Node_stack; }

namespace light {

class Prop_light : public Light {

public:

	void init(Prop* prop, uint32_t part);

	virtual const Entity_transformation& transformation_at(
			float time, Entity_transformation& transformation) const final override;

	virtual void sample(const Entity_transformation& transformation, float time,
						float3_p p, float3_p n, bool total_sphere,
						sampler::Sampler& sampler, Worker& worker,
						Sampler_filter filter, Sample& result) const override;

	virtual float pdf(const Entity_transformation& transformation,
					  float3_p p, float3_p wi, bool total_sphere,
					  Worker& worker, Sampler_filter filter) const override;

	virtual float3 power(const math::aabb& scene_bb) const final override;

	virtual void prepare_sampling() override;

	virtual bool equals(const Prop* prop, uint32_t part) const final override;

protected:

	Prop* prop_;

	uint32_t part_;
};

}}
