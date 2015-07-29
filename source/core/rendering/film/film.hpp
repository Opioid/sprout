#pragma once

#include "rendering/rectangle.hpp"
#include "image/typed_image.hpp"

namespace thread {

class Pool;

}

namespace sampler {

struct Camera_sample;

}

namespace rendering { namespace film {

namespace tonemapping {

class Tonemapper;

}

class Film {
public:

	Film(math::uint2 dimensions, float exposure, tonemapping::Tonemapper* tonemapper);
	virtual ~Film();

	math::uint2 dimensions() const;

	const image::Image_float_4& resolve(thread::Pool& pool);

	void clear();

//	float seed(uint32_t x, uint32_t y) const;
//	void set_seed(uint32_t x, uint32_t y, float seed);

	virtual void add_sample(const sampler::Camera_sample& sample, const math::float3& color, const Rectui& tile) = 0;

protected:

	void add_pixel(uint32_t x, uint32_t y, const math::float3& color, float weight);

	void add_pixel_atomic(uint32_t x, uint32_t y, const math::float3& color, float weight);

	struct Pixel {
		math::float3 color;
		float        weight_sum;
	};

	void resolve(uint32_t begin, uint32_t end);

	static math::float3 expose(const math::float3& color, float exposure);

	Pixel* pixels_;

	float exposure_;

	tonemapping::Tonemapper* tonemapper_;

	image::Image_float_4 image_;

	float* seeds_;
};

}}
