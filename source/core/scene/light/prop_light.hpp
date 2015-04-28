#pragma once

#include "light.hpp"

namespace scene {

class Prop;

namespace light {

class Prop_light : public Light {
public:

	void init(Prop* prop);

	virtual void sample(const math::float3& p, float time, uint32_t max_samples, sampler::Sampler& sampler,
						std::vector<Sample>& samples) const;

private:

	Prop* prop_;
};

}}
