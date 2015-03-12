#pragma once

#include "image/buffer/buffer4.hpp"

namespace sampler {

struct Camera_sample;

}

namespace film {

class Film {
public:

	Film(const math::uint2& dimensions);
	virtual ~Film();

	const math::uint2& dimensions() const;

	const image::Buffer4& resolve();

	virtual void add_sample(const sampler::Camera_sample& sample, const math::float3& color) = 0;

protected:

	void add_pixel(uint32_t x, uint32_t y, const math::float3& color, float weight);

	struct Pixel {
		math::float3 color;
		float        weight_sum;
	};

	Pixel* pixels_;

	image::Buffer4 image_buffer_;
};

}
