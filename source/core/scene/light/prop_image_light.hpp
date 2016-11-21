#pragma once

#include "prop_light.hpp"

namespace scene { namespace light {

class Prop_image_light : public Prop_light {

public:

	virtual void sample(const Transformation& transformation, float time,
						float3_p p, float3_p n, bool total_sphere,
						sampler::Sampler& sampler, uint32_t sampler_dimension, Worker& worker,
						Sampler_filter filter, Sample& result) const final override;

	virtual float pdf(const Transformation& transformation,
					  float3_p p, float3_p wi, bool total_sphere,
					  Worker& worker, Sampler_filter filter) const final override;

	virtual void prepare_sampling(uint32_t light_id, thread::Pool& pool) final override;
};

}}
