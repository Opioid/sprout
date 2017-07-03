#pragma once

#include "sensor.hpp"

namespace rendering { namespace sensor {

class Opaque : public Sensor {

public:

	Opaque(int2 dimensions, float exposure);
	virtual ~Opaque();

	virtual void clear() override final;

	virtual size_t num_bytes() const override final;

protected:

	virtual void add_pixel(int2 pixel, const float4& color, float weight) override final;

	virtual void add_pixel_atomic(int2 pixel, const float4& color, float weight) override final;

	virtual void resolve(int32_t begin, int32_t end, image::Float_4& target) const override final;

	// weight_sum is saved in pixel.w
	float4* pixels_;
};

}}
