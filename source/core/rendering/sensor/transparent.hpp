#pragma once

#include "sensor.hpp"

namespace rendering { namespace sensor {

class Transparent : public Sensor {

public:

	Transparent(int2 dimensions, float exposure);
	virtual ~Transparent();

	virtual void clear() override final;

	virtual size_t num_bytes() const override final;

protected:

	virtual void add_pixel(int2 pixel, const float4& color, float weight) override final;

	virtual void add_pixel_atomic(int2 pixel, const float4& color, float weight) override final;

	virtual void resolve(int32_t begin, int32_t end, image::Float4& target) const override final;

	struct Pixel {
		float4 color;
		float  weight_sum;
	};

	Pixel* pixels_;
};

}}
