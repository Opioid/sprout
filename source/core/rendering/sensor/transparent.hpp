#pragma once

#include "sensor.hpp"

namespace rendering { namespace sensor {

class Transparent : public Sensor {

public:

	Transparent(int2 dimensions, const tonemapping::Tonemapper* tonemapper);
	virtual ~Transparent();

	virtual void clear() final override;

protected:

	virtual void add_pixel(int2 pixel, const float4& color, float weight) final override;

	virtual void add_pixel_atomic(int2 pixel, const float4& color, float weight) final override;

	virtual void resolve(int32_t begin, int32_t end,
						 image::Image_float_4& target) const final override;

	struct Pixel {
		float4 color;
		float  weight_sum;
	};

	Pixel* pixels_;
};


}}
