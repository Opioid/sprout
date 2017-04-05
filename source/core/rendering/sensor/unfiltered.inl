#pragma once

#include "unfiltered.hpp"
#include "sampler/camera_sample.hpp"

namespace rendering { namespace sensor {

template<class Base, class Clamp>
Unfiltered<Base, Clamp>::Unfiltered(int2 dimensions, float exposure, const Clamp& clamp) :
	Base(dimensions, exposure), clamp_(clamp) {}

template<class Base, class Clamp>
int32_t Unfiltered<Base, Clamp>::filter_radius_int() const {
	return 0;
}

template<class Base, class Clamp>
void Unfiltered<Base, Clamp>::add_sample(const sampler::Camera_sample& sample, const float4& color,
										 const int4& /*tile*/, const int4& bounds) {
	Base::add_pixel(bounds.xy() + sample.pixel, clamp_.clamp(color), 1.f);
}

}}
