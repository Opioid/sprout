#pragma once

#include "light.hpp"
#include <memory>

namespace scene {

namespace shape {

class Shape;

}

namespace light {

class Shape_light : public Light {
public:

	void init(std::shared_ptr<shape::Shape> shape);

	virtual void sample(const math::float3& p, float time, uint32_t max_samples, sampler::Sampler& sampler,
						std::vector<Sample>& samples) const;

	void set_color(const math::float3& color);
	void set_lumen(float lumen);

private:

	std::shared_ptr<shape::Shape> shape_;

	math::float3 color_;
	float lumen_;
};

}}
