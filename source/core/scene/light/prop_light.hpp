#pragma once

#include "light.hpp"

namespace scene {

class Prop;

namespace light {

class Prop_light : public Light {
public:

	void init(Prop* prop, uint32_t part = 0);

	virtual void sample(const math::float3& p, float time, uint32_t max_samples, sampler::Sampler& sampler,
						std::vector<Sample>& samples) const;

	virtual math::float3 energy(const math::aabb& scene_bb) const final override;

	virtual void prepare_sampling() final override;

private:

	Prop* prop_;

	uint32_t part_;

	float area_;
};

}}
