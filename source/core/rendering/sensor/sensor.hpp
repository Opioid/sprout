#pragma once

#include "image/typed_image.hpp"
#include "base/math/rectangle.hpp"
#include <memory>

namespace thread { class Pool; }

namespace sampler { struct Camera_sample; }

namespace rendering { namespace sensor {

namespace tonemapping { class Tonemapper; }

class Sensor {
public:

	Sensor(math::int2 dimensions, float exposure, std::unique_ptr<tonemapping::Tonemapper> tonemapper);
	virtual ~Sensor();

	math::int2 dimensions() const;

	const image::Image_float_4& resolve(thread::Pool& pool);

	virtual void clear() = 0;

	virtual void add_sample(const sampler::Camera_sample& sample, const math::float4& color, const math::Recti& tile, const math::Recti& bounds) = 0;

protected:

	virtual void add_pixel(int32_t x, int32_t y, const math::float4& color, float weight) = 0;

	virtual void add_pixel_atomic(int32_t x, int32_t y, const math::float4& color, float weight) = 0;

	virtual void resolve(int32_t begin, int32_t end) = 0;

	static math::float3 expose(const math::float3& color, float exposure);

	float exposure_;

	std::unique_ptr<tonemapping::Tonemapper> tonemapper_;

	image::Image_float_4 image_;
};

}}
