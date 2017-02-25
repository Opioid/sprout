#pragma once

#include "filtered.hpp"
#include "filter/sensor_filter.hpp"
#include "sampler/camera_sample.hpp"

#include <iostream>

namespace rendering { namespace sensor {

template<class Base, class Clamp>
Filtered<Base, Clamp>::Filtered(int2 dimensions, float exposure, const Clamp& clamp,
								const filter::Filter* filter) :
	Base(dimensions, exposure), clamp_(clamp), filter_(filter) {}

template<class Base, class Clamp>
Filtered<Base, Clamp>::~Filtered() {
	delete filter_;
}

template<class Base, class Clamp>
int32_t Filtered<Base, Clamp>::filter_radius_int() const {
	return static_cast<int32_t>(filter_->radius() + 0.5f);
}

template<class Base, class Clamp>
void Filtered<Base, Clamp>::add_sample(const sampler::Camera_sample& sample, float4_p color,
									   const int4& tile, const int4& bounds) {
	float4 clamped_color = clamp_.clamp(color);

	int32_t x = bounds.x + sample.pixel.x;
	int32_t y = bounds.y + sample.pixel.y;

	float ox = sample.pixel_uv.x - 0.5f;
	float oy = sample.pixel_uv.y - 0.5f;

	float wx0 = filter_->evaluate(ox + 1.f);
	float wx1 = filter_->evaluate(ox);
	float wx2 = filter_->evaluate(ox - 1.f);

	float wy0 = filter_->evaluate(oy + 1.f);
	float wy1 = filter_->evaluate(oy);
	float wy2 = filter_->evaluate(oy - 1.f);

	// 1. row
	add_weighted_pixel(int2(x - 1, y - 1), wx0 * wy0, clamped_color, tile, bounds);
	add_weighted_pixel(int2(x, y - 1),	   wx1 * wy0, clamped_color, tile, bounds);
	add_weighted_pixel(int2(x + 1, y - 1), wx2 * wy0, clamped_color, tile, bounds);

	// 2. row
	add_weighted_pixel(int2(x - 1, y), wx0 * wy1, clamped_color, tile, bounds);
	add_weighted_pixel(int2(x, y),	   wx1 * wy1, clamped_color, tile, bounds);
	add_weighted_pixel(int2(x + 1, y), wx2 * wy1, clamped_color, tile, bounds);

	// 3. row
	add_weighted_pixel(int2(x - 1, y + 1), wx0 * wy2, clamped_color, tile, bounds);
	add_weighted_pixel(int2(x, y + 1),	   wx1 * wy2, clamped_color, tile, bounds);
	add_weighted_pixel(int2(x + 1, y + 1), wx2 * wy2, clamped_color, tile, bounds);


	/*
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
*/
}

template<class Base, class Clamp>
void Filtered<Base, Clamp>::add_weighted_pixel(int2 pixel, float weight, float4_p color,
											   const int4& tile, const int4& bounds) {
    if (pixel.x < bounds.x || pixel.y < bounds.y
    ||  bounds.z < pixel.x || bounds.w < pixel.y) {
        return;
    }

    if ((pixel.x >= tile.z - 1 && pixel.x < bounds.z)
    ||  (pixel.y >= tile.w - 1 && pixel.y < bounds.w)
	||  (pixel.x <= tile.x	   && pixel.x > bounds.x)
	||  (pixel.y <= tile.y     && pixel.y > bounds.y)) {
		Base::add_pixel_atomic(pixel, color, weight);
	} else {
		Base::add_pixel(pixel, color, weight);
	}
}

template<class Base, class Clamp>
void Filtered<Base, Clamp>::weight_and_add_pixel(int2 pixel, float2 relative_offset,
												 float4_p color,
												 const int4& tile, const int4& bounds) {
    if (pixel.x < bounds.x || pixel.y < bounds.y
    ||  bounds.z < pixel.x || bounds.w < pixel.y) {
        return;
    }

	float weight = filter_->evaluate(relative_offset);

    if ((pixel.x >= tile.z - 1 && pixel.x < bounds.z)
    ||  (pixel.y >= tile.w - 1 && pixel.y < bounds.w)
	||  (pixel.x <= tile.x     && pixel.x > bounds.x)
	||  (pixel.y <= tile.y     && pixel.y > bounds.y)) {
		Base::add_pixel_atomic(pixel, color, weight);
	} else {
		Base::add_pixel(pixel, color, weight);
	}
}

}}
