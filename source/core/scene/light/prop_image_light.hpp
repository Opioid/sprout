#pragma once

#include "prop_light.hpp"

namespace scene { namespace light {

class Prop_image_light : public Prop_light {

public:

	virtual bool sample(const Transformation& transformation,
						const float3& p, const float3& n, float time, bool total_sphere,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Sampler_filter filter, Worker& worker, Sample& result) const override final;

	virtual float pdf(const Ray& ray, const Intersection& intersection, bool total_sphere,
					  Sampler_filter filter, Worker& worker) const override final;

	virtual void prepare_sampling(uint32_t light_id, thread::Pool& pool) override final;
};

}}
