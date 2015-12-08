#pragma once

#include "sensor.hpp"

namespace rendering { namespace sensor {

class Transparent : public Sensor {
public:

	Transparent(math::uint2 dimensions, float exposure, std::unique_ptr<tonemapping::Tonemapper> tonemapper);
	virtual ~Transparent();

	virtual void clear() final override;

protected:

	virtual void on_resize(math::uint2 dimensions) final override;

	virtual void add_pixel(uint32_t x, uint32_t y, const math::float4& color, float weight) final override;

	virtual void add_pixel_atomic(uint32_t x, uint32_t y, const math::float4& color, float weight) final override;

	virtual void resolve(uint32_t begin, uint32_t end) final override;

	struct Pixel {
		math::float4 color;
		float        weight_sum;
	};

	Pixel* pixels_;
};


}}
