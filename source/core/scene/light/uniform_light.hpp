#pragma once

#include "light.hpp"

namespace scene { namespace light {

class Uniform_light : public Light {
public:

	void init(const math::float3& energy);

	virtual void transformation_at(float time, Composed_transformation& transformation) const final override;

	virtual void sample(const Composed_transformation& transformation, const math::float3& p, const math::float3& n,
						sampler::Sampler& sampler, uint32_t max_samples, std::vector<Sample>& samples) const;

	virtual math::float3 evaluate(const math::float3& wi) const final override;

	virtual float pdf(const Composed_transformation& transformation, const math::float3& p, const math::float3& wi) const final override;

	virtual math::float3 power(const math::aabb& scene_bb) const final override;

	virtual void prepare_sampling() final override;

	virtual bool equals(const Prop* prop, uint32_t part) const final override;

private:

	math::float3 energy_;
};

}}
