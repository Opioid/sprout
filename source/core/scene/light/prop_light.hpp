#pragma once

#include "light.hpp"

namespace scene {

class Prop;

namespace light {

class Prop_light : public Light {
public:

	void init(Prop* prop, uint32_t part = 0);

	virtual void transformation_at(float time, Composed_transformation& transformation) const final override;

	virtual void sample(const Composed_transformation& transformation, const math::float3& p, uint32_t max_samples, sampler::Sampler& sampler,
						std::vector<Sample>& samples) const;

	virtual float pdf(const Composed_transformation& transformation, const math::float3& p, const math::float3& wi) const final override;

	virtual math::float3 energy(const math::aabb& scene_bb) const final override;

	virtual void prepare_sampling() final override;

	virtual bool equals(const Prop* prop, uint32_t part) const final override;

private:

	Prop* prop_;

	uint32_t part_;

	float area_;
};

}}
