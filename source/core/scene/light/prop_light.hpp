#pragma once

#include "light.hpp"

namespace scene {

namespace shape { class Node_stack; }

class Prop;

namespace light {

class Prop_light : public Light {
public:

	void init(Prop* prop, uint32_t part);

	virtual const entity::Composed_transformation& transformation_at(
			float time, entity::Composed_transformation& transformation) const final override;

	virtual void sample(const entity::Composed_transformation& transformation, float time,
						const math::vec3& p, const math::vec3& n, bool total_sphere,
						const image::texture::sampler::Sampler_2D& image_sampler,
						sampler::Sampler& sampler, shape::Node_stack& node_stack, Sample& result) const override;

	virtual float pdf(const entity::Composed_transformation& transformation,
					  const math::vec3& p, const math::vec3& wi, bool total_sphere,
					  const image::texture::sampler::Sampler_2D& image_sampler,
					  shape::Node_stack& node_stack) const override;

	virtual math::vec3 power(const math::aabb& scene_bb) const final override;

	virtual void prepare_sampling() override;

	virtual bool equals(const Prop* prop, uint32_t part) const final override;

protected:

	Prop* prop_;

	uint32_t part_;

	float area_;
};

}}
