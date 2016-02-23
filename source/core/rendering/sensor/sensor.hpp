#pragma once

#include "image/typed_image.hpp"
#include "base/math/rectangle.hpp"

namespace thread { class Pool; }

namespace sampler { struct Camera_sample; }

namespace rendering { namespace sensor {

namespace tonemapping { class Tonemapper; }

class Sensor {
public:

	Sensor(math::int2 dimensions, float exposure, const tonemapping::Tonemapper* tonemapper);
	virtual ~Sensor();

	math::int2 dimensions() const;

	void resolve(thread::Pool& pool, image::Image_float_4& target);

	virtual int32_t filter_radius_int() const = 0;

	virtual void clear() = 0;

	virtual void add_sample(const sampler::Camera_sample& sample, const math::float4& color,
							const math::Recti& tile, const math::Recti& view_bounds) = 0;

protected:

	virtual void add_pixel(math::int2 pixel, const math::float4& color, float weight) = 0;

	virtual void add_pixel_atomic(math::int2 pixel, const math::float4& color, float weight) = 0;

	virtual void resolve(int32_t begin, int32_t end, image::Image_float_4& target) = 0;

	math::vec3 expose(math::pvec3 color) const;

	math::int2 dimensions_;

	const float exp2_exposure_;

	const tonemapping::Tonemapper* tonemapper_;
};

}}
