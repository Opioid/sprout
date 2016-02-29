#pragma once

#include "sensor.hpp"

namespace rendering { namespace sensor {

class Opaque : public Sensor {
public:

	Opaque(math::int2 dimensions, const tonemapping::Tonemapper* tonemapper);
	virtual ~Opaque();

	virtual void clear() final override;

protected:

	virtual void add_pixel(math::int2 pixel, const math::float4& color, float weight) final override;

	virtual void add_pixel_atomic(math::int2 pixel, const math::float4& color, float weight) final override;

	virtual void resolve(int32_t begin, int32_t end, image::Image_float_4& target) const final override;

	// weight_sum is saved in pixel.w
	math::float4* pixels_;
};

}}
