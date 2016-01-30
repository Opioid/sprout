#pragma once

#include "sensor.hpp"

namespace rendering { namespace sensor {

class Opaque : public Sensor {
public:

	Opaque(math::int2 dimensions, float exposure, const tonemapping::Tonemapper* tonemapper);
	virtual ~Opaque();

	virtual void clear() final override;

protected:

	virtual void add_pixel(math::int2 pixel, const math::float4& color, float weight) final override;

	virtual void add_pixel_atomic(math::int2 pixel, const math::float4& color, float weight) final override;

	virtual void resolve(int32_t begin, int32_t end) final override;

	struct Pixel {
		math::float3 color;
		float        weight_sum;
	};

	Pixel* pixels_;
};

}}
