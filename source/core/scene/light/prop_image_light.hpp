#ifndef SU_SCENE_LIGHT_PROP_IMAGE_LIGHT_HPP
#define SU_SCENE_LIGHT_PROP_IMAGE_LIGHT_HPP

#include "prop_light.hpp"

namespace scene::light {

class Prop_image_light : public Prop_light {

public:

	virtual bool sample(const Transformation& transformation,
						const float3& p, float time,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Sampler_filter filter, const Worker& worker,
						Sample& result) const override final;

	virtual bool sample(const Transformation& transformation,
						const float3& p, const float3& n, float time, bool total_sphere,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Sampler_filter filter, const Worker& worker,
						Sample& result) const override final;

	virtual float pdf(const Ray& ray, const Intersection& intersection, bool total_sphere,
					  Sampler_filter filter, const Worker& worker) const override final;

	virtual void prepare_sampling(uint32_t light_id, thread::Pool& pool) override final;
};

}

#endif
