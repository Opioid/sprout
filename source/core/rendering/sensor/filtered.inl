#pragma once

#include "filtered.hpp"
#include "filter/filter.hpp"
#include "sampler/camera_sample.hpp"

namespace rendering { namespace sensor {

template<class Base, class Clamp>
Filtered<Base, Clamp>::Filtered(int2 dimensions,
								float exposure,
								const tonemapping::Tonemapper* tonemapper,
								const Clamp& clamp,
								const filter::Filter* filter) :
	Base(dimensions, exposure, tonemapper), clamp_(clamp), filter_(filter) {}

template<class Base, class Clamp>
Filtered<Base, Clamp>::~Filtered() {
	delete filter_;
}

template<class Base, class Clamp>
int32_t Filtered<Base, Clamp>::filter_radius_int() const {
	return static_cast<int32_t>(filter_->radius() + 0.5f);
}

template<class Base, class Clamp>
void Filtered<Base, Clamp>::add_sample(const sampler::Camera_sample& sample, const float4& color,
									   const math::Recti& tile, const math::Recti& bounds) {
	float4 clamped_color = clamp_.clamp(color);

	int32_t x = bounds.start.x + sample.pixel.x;
	int32_t y = bounds.start.y + sample.pixel.y;

	float ox = sample.pixel_uv.x - 0.5f;
	float oy = sample.pixel_uv.y - 0.5f;

	weight_and_add_pixel(int2(x - 1, y - 1), float2(ox + 1.f, oy + 1.f),
						 clamped_color, tile, bounds);

	weight_and_add_pixel(int2(x, y - 1), float2(ox, oy + 1.f),
						 clamped_color, tile, bounds);

	weight_and_add_pixel(int2(x + 1, y - 1), float2(ox - 1.f, oy + 1.f),
						 clamped_color, tile, bounds);

	weight_and_add_pixel(int2(x - 1, y), float2(ox + 1.f, oy),
						 clamped_color, tile, bounds);

	// center
	weight_and_add_pixel(int2(x, y), float2(ox, oy),
						 clamped_color, tile, bounds);

	weight_and_add_pixel(int2(x + 1, y), float2(ox - 1.f, oy),
						 clamped_color, tile, bounds);

	weight_and_add_pixel(int2(x - 1, y + 1), float2(ox + 1.f, oy - 1.f),
						 clamped_color, tile, bounds);

	weight_and_add_pixel(int2(x, y + 1), float2(ox, oy - 1.f),
						 clamped_color, tile, bounds);

	weight_and_add_pixel(int2(x + 1, y + 1), float2(ox - 1.f, oy - 1.f),
						 clamped_color, tile, bounds);
}

template<class Base, class Clamp>
void Filtered<Base, Clamp>::weight_and_add_pixel(int2 pixel, float2 relative_offset,
												 const float4& color,
												 const math::Recti& tile,
												 const math::Recti& bounds) {
	if (pixel.x < bounds.start.x || pixel.x >= bounds.end.x
	||  pixel.y < bounds.start.y || pixel.y >= bounds.end.y) {
		return;
	}

	float weight = filter_->evaluate(relative_offset);

	if ((pixel.x >= tile.end.x - 1 && pixel.x < bounds.end.x - 1)
	||  (pixel.y >= tile.end.y - 1 && pixel.y < bounds.end.y - 1)
	||  (pixel.x <= tile.start.x   && pixel.x > bounds.start.x)
	||  (pixel.y <= tile.start.y   && pixel.y > bounds.start.y)) {
		Base::add_pixel_atomic(pixel, color, weight);
	} else {
		Base::add_pixel(pixel, color, weight);
	}
}

}}
