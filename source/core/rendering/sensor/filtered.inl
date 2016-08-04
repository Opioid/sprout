#pragma once

#include "filtered.hpp"
#include "filter/filter.hpp"
#include "sampler/camera_sample.hpp"

namespace rendering { namespace sensor {

template<class Base, class Clamp>
Filtered<Base, Clamp>::Filtered(int2 dimensions,
								const tonemapping::Tonemapper* tonemapper,
								const Clamp& clamp,
								const filter::Filter* filter) :
	Base(dimensions, tonemapper), clamp_(clamp), filter_(filter) {}

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
									   const math::Recti& tile, const math::Recti& view_bounds) {
	math::Recti view_tile{view_bounds.start + tile.start, view_bounds.start + tile.end};

	float4 clamped_color = clamp_.clamp(color);

	int32_t x = view_bounds.start.x + sample.pixel.x;
	int32_t y = view_bounds.start.y + sample.pixel.y;

	float ox = sample.pixel_uv.x - 0.5f;
	float oy = sample.pixel_uv.y - 0.5f;

	weight_and_add_pixel(int2(x - 1, y - 1), float2(ox + 1.f, oy + 1.f),
						 clamped_color, view_tile, view_bounds);

	weight_and_add_pixel(int2(x, y - 1), float2(ox, oy + 1.f),
						 clamped_color, view_tile, view_bounds);

	weight_and_add_pixel(int2(x + 1, y - 1), float2(ox - 1.f, oy + 1.f),
						 clamped_color, view_tile, view_bounds);

	weight_and_add_pixel(int2(x - 1, y), float2(ox + 1.f, oy),
						 clamped_color, view_tile, view_bounds);

	// center
	weight_and_add_pixel(int2(x, y), float2(ox, oy),
						 clamped_color, view_tile, view_bounds);

	weight_and_add_pixel(int2(x + 1, y), float2(ox - 1.f, oy),
						 clamped_color, view_tile, view_bounds);

	weight_and_add_pixel(int2(x - 1, y + 1), float2(ox + 1.f, oy - 1.f),
						 clamped_color, view_tile, view_bounds);

	weight_and_add_pixel(int2(x, y + 1), float2(ox, oy - 1.f),
						 clamped_color, view_tile, view_bounds);

	weight_and_add_pixel(int2(x + 1, y + 1), float2(ox - 1.f, oy - 1.f),
						 clamped_color, view_tile, view_bounds);
}

template<class Base, class Clamp>
void Filtered<Base, Clamp>::weight_and_add_pixel(int2 pixel, float2 relative_offset,
												 const float4& color,
												 const math::Recti& view_tile,
												 const math::Recti& view_bounds) {
	if (pixel.x < view_bounds.start.x || pixel.x >= view_bounds.end.x
	||  pixel.y < view_bounds.start.y || pixel.y >= view_bounds.end.y) {
		return;
	}

	float weight = filter_->evaluate(relative_offset);

	if ((pixel.x >= view_tile.end.x - 1 && pixel.x < view_bounds.end.x - 1)
	||  (pixel.y >= view_tile.end.y - 1 && pixel.y < view_bounds.end.y - 1)
	||  (pixel.x <= view_tile.start.x   && pixel.x > view_bounds.start.x)
	||  (pixel.y <= view_tile.start.y   && pixel.y > view_bounds.start.y)) {
		Base::add_pixel_atomic(pixel, color, weight);
	} else {
		Base::add_pixel(pixel, color, weight);
	}
}

}}
