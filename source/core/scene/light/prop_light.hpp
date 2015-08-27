#pragma once

#include "light.hpp"

namespace scene {

class Prop;

namespace light {

class Prop_light : public Light {
public:

	void init(Prop* prop, uint32_t part = 0);

	virtual void transformation_at(float time, entity::Composed_transformation& transformation) const final override;

	virtual void sample(const entity::Composed_transformation& transformation,
						const math::float3& p, const math::float3& n, bool ignore_transmission,
						const image::texture::sampler::Sampler_2D& image_sampler, sampler::Sampler& sampler,
						uint32_t max_samples, std::vector<Sample>& samples) const override;

	virtual math::float3 evaluate(const math::float3& wi) const final override;

	virtual float pdf(const entity::Composed_transformation& transformation,
					  const math::float3& p, const math::float3& wi,
					  const image::texture::sampler::Sampler_2D& image_sampler) const override;

	virtual math::float3 power(const math::aabb& scene_bb) const final override;

	virtual void prepare_sampling() override;

	virtual bool equals(const Prop* prop, uint32_t part) const final override;

protected:

	Prop* prop_;

	uint32_t part_;

	float area_;
};

}}
