#pragma once

#include "unfiltered.hpp"
#include "sampler/camera_sample.hpp"

namespace rendering { namespace film {

template<class Base>
Unfiltered<Base>::Unfiltered(const math::uint2& dimensions, float exposure, tonemapping::Tonemapper* tonemapper) :
	Base(dimensions, exposure, tonemapper) {}

template<class Base>
void Unfiltered<Base>::add_sample(const sampler::Camera_sample& sample, const math::float4& color,
								  const Rectui& /*tile*/) {
	Base::add_pixel(sample.pixel.x, sample.pixel.y, color, 1.f);
}

}}
