#pragma once

#include "filtered.hpp"
#include "filter/sensor_filter.hpp"
#include "sampler/camera_sample.hpp"
#include "base/math/vector4.inl"

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
int4 Filtered<Base, Clamp>::isolated_tile(const int4& tile) const {
	const int32_t r = filter_radius_int();
	return tile + int4(r, r, -r, -r);
}

template<class Base, class Clamp>
void Filtered<Base, Clamp>::add_sample(const sampler::Camera_sample& sample, const float4& color,
									   const int4& isolated_tile, const int4& bounds) {
	const float4 clamped_color = clamp_.clamp(color);

	const int32_t x = bounds[0] + sample.pixel[0];
	const int32_t y = bounds[1] + sample.pixel[1];

	const float ox = sample.pixel_uv[0] - 0.5f;
	const float oy = sample.pixel_uv[1] - 0.5f;

	const float wx0 = filter_->evaluate(ox + 1.f);
	const float wx1 = filter_->evaluate(ox);
	const float wx2 = filter_->evaluate(ox - 1.f);

	const float wy0 = filter_->evaluate(oy + 1.f);
	const float wy1 = filter_->evaluate(oy);
	const float wy2 = filter_->evaluate(oy - 1.f);

	// 1. row
	add_weighted_pixel(int2(x - 1, y - 1), wx0 * wy0, clamped_color, isolated_tile, bounds);
	add_weighted_pixel(int2(x, y - 1),	   wx1 * wy0, clamped_color, isolated_tile, bounds);
	add_weighted_pixel(int2(x + 1, y - 1), wx2 * wy0, clamped_color, isolated_tile, bounds);

	// 2. row
	add_weighted_pixel(int2(x - 1, y), wx0 * wy1, clamped_color, isolated_tile, bounds);
	add_weighted_pixel(int2(x, y),	   wx1 * wy1, clamped_color, isolated_tile, bounds);
	add_weighted_pixel(int2(x + 1, y), wx2 * wy1, clamped_color, isolated_tile, bounds);

	// 3. row
	add_weighted_pixel(int2(x - 1, y + 1), wx0 * wy2, clamped_color, isolated_tile, bounds);
	add_weighted_pixel(int2(x, y + 1),	   wx1 * wy2, clamped_color, isolated_tile, bounds);
	add_weighted_pixel(int2(x + 1, y + 1), wx2 * wy2, clamped_color, isolated_tile, bounds);

	/*
	weight_and_add_pixel(int2(x - 1, y - 1), float2(ox + 1.f, oy + 1.f),
						 clamped_color, isolated_tile, bounds);

	weight_and_add_pixel(int2(x, y - 1), float2(ox, oy + 1.f),
						 clamped_color, isolated_tile, bounds);

	weight_and_add_pixel(int2(x + 1, y - 1), float2(ox - 1.f, oy + 1.f),
						 clamped_color, isolated_tile, bounds);

	weight_and_add_pixel(int2(x - 1, y), float2(ox + 1.f, oy),
						 clamped_color, isolated_tile, bounds);

	// center
	weight_and_add_pixel(int2(x, y), float2(ox, oy),
						 clamped_color, isolated_tile, bounds);

	weight_and_add_pixel(int2(x + 1, y), float2(ox - 1.f, oy),
						 clamped_color, isolated_tile, bounds);

	weight_and_add_pixel(int2(x - 1, y + 1), float2(ox + 1.f, oy - 1.f),
						 clamped_color, isolated_tile, bounds);

	weight_and_add_pixel(int2(x, y + 1), float2(ox, oy - 1.f),
						 clamped_color, isolated_tile, bounds);

	weight_and_add_pixel(int2(x + 1, y + 1), float2(ox - 1.f, oy - 1.f),
						 clamped_color, isolated_tile, bounds);
*/
}

template<class Base, class Clamp>
void Filtered<Base, Clamp>::add_weighted_pixel(int2 pixel, float weight, const float4& color,
											   const int4& isolated_tile, const int4& bounds) {
	if (pixel[0] < bounds[0] || pixel[1] < bounds[1]
	||  bounds[2] < pixel[0] || bounds[3] < pixel[1]) {
		return;
	}

	if ((pixel[0] < isolated_tile[0])
	||  (pixel[1] < isolated_tile[1])
	||	(pixel[0] > isolated_tile[2])
	||  (pixel[1] > isolated_tile[3])) {
		Base::add_pixel_atomic(pixel, color, weight);
	} else {
		Base::add_pixel(pixel, color, weight);
	}
}

template<class Base, class Clamp>
void Filtered<Base, Clamp>::weight_and_add_pixel(int2 pixel, float2 relative_offset,
												 const float4& color,
												 const int4& isolated_tile, const int4& bounds) {
	if (pixel[0] < bounds[0] || pixel[1] < bounds[1]
	||  bounds[2] < pixel[0] || bounds[3] < pixel[1]) {
        return;
    }

	float weight = filter_->evaluate(relative_offset);

	if ((pixel[0] < isolated_tile[0])
	||  (pixel[1] < isolated_tile[1])
	||	(pixel[0] > isolated_tile[2])
	||  (pixel[1] > isolated_tile[3])) {
		Base::add_pixel_atomic(pixel, color, weight);
	} else {
		Base::add_pixel(pixel, color, weight);
	}
}

}}
