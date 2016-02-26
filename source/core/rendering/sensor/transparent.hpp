#pragma once

#include "sensor.hpp"

namespace rendering { namespace sensor {

class Transparent : public Sensor {
public:

	Transparent(math::int2 dimensions, float exposure, const tonemapping::Tonemapper* tonemapper);
	virtual ~Transparent();

	virtual void clear() final override;

protected:

	virtual void add_pixel(math::int2 pixel, const math::float4& color, float weight) final override;

	virtual void add_pixel_atomic(math::int2 pixel, const math::float4& color, float weight) final override;

	virtual void resolve(int32_t begin, int32_t end, image::Image_float_4& target) const final override;

	struct Pixel {
		math::float4 color;
		float        weight_sum;
	};

	Pixel* pixels_;
};


}}
