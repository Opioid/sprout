#ifndef SU_CORE_SCENE_LIGHT_NULL_LIGHT_HPP
#define SU_CORE_SCENE_LIGHT_NULL_LIGHT_HPP

#include "light.hpp"

namespace scene {

namespace shape { class Node_stack; }

namespace light {

class Null_light : public Light {

public:

	virtual Transformation const& transformation_at(
			float time, Transformation& transformation) const override final;

	virtual bool sample(f_float3 p, float time, Transformation const& transformation,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Sampler_filter filter, Worker const& worker,
						Sample& result) const override final;

	virtual bool sample(f_float3 p, f_float3 n,
						float time, Transformation const& transformation, bool total_sphere,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Sampler_filter filter, Worker const& worker,
						Sample& result) const override final;

	virtual float pdf(Ray const& ray, const Intersection& intersection, bool total_sphere,
					  Sampler_filter filter, Worker const& worker) const override final;

	virtual float3 power(math::AABB const& scene_bb) const override final;

	virtual void prepare_sampling(uint32_t light_id, thread::Pool& pool) override;

	virtual bool equals(const Prop* prop, uint32_t part) const override final;
};

}}

#endif
