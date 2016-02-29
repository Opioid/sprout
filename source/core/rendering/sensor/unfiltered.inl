#pragma once

#include "unfiltered.hpp"
#include "sampler/camera_sample.hpp"

namespace rendering { namespace sensor {

template<class Base, class Clamp>
Unfiltered<Base, Clamp>::Unfiltered(math::int2 dimensions,
									const tonemapping::Tonemapper* tonemapper,
									const Clamp& clamp) :
	Base(dimensions, tonemapper), clamp_(clamp) {}

template<class Base, class Clamp>
int32_t Unfiltered<Base, Clamp>::filter_radius_int() const {
	return 0;
}

template<class Base, class Clamp>
void Unfiltered<Base, Clamp>::add_sample(const sampler::Camera_sample& sample, const math::float4& color,
										 const math::Recti& /*tile*/, const math::Recti& view_bounds) {
	Base::add_pixel(view_bounds.start + sample.pixel, clamp_.clamp(color), 1.f);
}

}}
