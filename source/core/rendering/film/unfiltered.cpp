#include "unfiltered.hpp"
#include "rendering/sampler/camera_sample.hpp"

namespace rendering { namespace film {

Unfiltered::Unfiltered(const math::uint2& dimensions, float exposure, tonemapping::Tonemapper* tonemapper) :
	Film(dimensions, exposure, tonemapper) {}

void Unfiltered::add_sample(const sampler::Camera_sample& sample, const math::float3& color) {
	uint32_t x = static_cast<uint32_t>(sample.coordinates.x);
	uint32_t y = static_cast<uint32_t>(sample.coordinates.y);

	add_pixel(x, y, color, 1.f);
}

}}
