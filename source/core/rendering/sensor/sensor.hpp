#pragma once

#include "rendering/rectangle.hpp"
#include "image/typed_image.hpp"
#include <memory>

namespace thread { class Pool; }

namespace sampler { struct Camera_sample; }

namespace rendering { namespace sensor {

namespace tonemapping { class Tonemapper; }

class Sensor {
public:

	Sensor(math::uint2 dimensions, float exposure, std::unique_ptr<tonemapping::Tonemapper> tonemapper);
	virtual ~Sensor();

	math::uint2 dimensions() const;

	void resize(math::uint2 dimensions);

	const image::Image_float_4& resolve(thread::Pool& pool);

	math::uint2 seed(uint32_t x, uint32_t y) const;
	void set_seed(uint32_t x, uint32_t y, math::uint2 seed);

	virtual void clear() = 0;

	virtual void add_sample(const sampler::Camera_sample& sample, const math::float4& color, const Rectui& tile) = 0;

protected:

	virtual void on_resize(math::uint2 dimensions) = 0;

	virtual void add_pixel(uint32_t x, uint32_t y, const math::float4& color, float weight) = 0;

	virtual void add_pixel_atomic(uint32_t x, uint32_t y, const math::float4& color, float weight) = 0;

	virtual void resolve(uint32_t begin, uint32_t end) = 0;

	static math::float3 expose(const math::float3& color, float exposure);

	float exposure_;

	std::unique_ptr<tonemapping::Tonemapper> tonemapper_;

	image::Image_float_4 image_;

	math::uint2* seeds_;
};

}}
