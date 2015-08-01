#include "unfiltered.hpp"
#include "sampler/camera_sample.hpp"

namespace rendering { namespace film {

Unfiltered::Unfiltered(const math::uint2& dimensions, float exposure, tonemapping::Tonemapper* tonemapper) :
	Film(dimensions, exposure, tonemapper) {}

void Unfiltered::add_sample(const sampler::Camera_sample& sample, const math::float3& color, const Rectui& /*tile*/) {
	add_pixel(sample.pixel.x, sample.pixel.y, color, 1.f);
}

}}
