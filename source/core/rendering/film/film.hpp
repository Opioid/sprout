#pragma once

#include "rendering/rectangle.hpp"
#include "image/buffer/buffer4.hpp"

namespace rendering {

namespace sampler {

struct Camera_sample;

}

namespace film {

namespace tonemapping {

class Tonemapper;

}

class Film {
public:

	Film(const math::uint2& dimensions, float exposure, tonemapping::Tonemapper* tonemapper);
	virtual ~Film();

	const math::uint2& dimensions() const;

	const image::Buffer4& resolve();

	void clear();

	virtual void add_sample(const sampler::Camera_sample& sample, const math::float3& color, const Rectui& tile) = 0;

protected:

	void add_pixel(uint32_t x, uint32_t y, const math::float3& color, float weight);

	void add_pixel_atomic(uint32_t x, uint32_t y, const math::float3& color, float weight);

	static math::float3 expose(const math::float3& color, float exposure);

	struct Pixel {
		math::float3 color;
		float        weight_sum;
	};

	Pixel* pixels_;

	float exposure_;

	tonemapping::Tonemapper* tonemapper_;

	image::Buffer4 image_buffer_;
};

}}
