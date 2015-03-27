#pragma once

#include "light.hpp"

namespace scene { namespace light {

class Shape_light : public Light {
public:

	virtual void sample(const math::float3& p, float time, uint32_t max_samples, rendering::sampler::Sampler& sampler,
						std::vector<Sample>& samples) const;

	void set_color(const math::float3& color);
	void set_lumen(float lumen);

private:

	math::float3 color_;
	float lumen_;
};

}}
