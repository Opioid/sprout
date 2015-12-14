#pragma once

#include "unfiltered.hpp"
#include "sampler/camera_sample.hpp"

namespace rendering { namespace sensor {

template<class Base, class Clamp>
Unfiltered<Base, Clamp>::Unfiltered(math::int2 dimensions, float exposure,
									std::unique_ptr<tonemapping::Tonemapper> tonemapper, const Clamp& clamp) :
	Base(dimensions, exposure, std::move(tonemapper)), clamp_(clamp) {}

template<class Base, class Clamp>
void Unfiltered<Base, Clamp>::add_sample(const sampler::Camera_sample& sample, const math::float4& color,
										 const math::Recti& /*tile*/, const math::Recti& /*bounds*/) {
	Base::add_pixel(sample.pixel.x, sample.pixel.y, clamp_.clamp(color), 1.f);
}

}}
