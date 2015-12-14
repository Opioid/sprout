#pragma once

#include "filtered.hpp"
#include "sampler/camera_sample.hpp"

namespace rendering { namespace sensor {

template<class Base, class Clamp, class Filter>
Filtered<Base, Clamp, Filter>::Filtered(const math::int2& dimensions, float exposure,
										std::unique_ptr<tonemapping::Tonemapper> tonemapper, const Clamp& clamp,
										std::unique_ptr<Filter> filter) :
	Base(dimensions, exposure, std::move(tonemapper)), clamp_(clamp), filter_(std::move(filter)) {}

template<class Base, class Clamp, class Filter>
Filtered<Base, Clamp, Filter>::~Filtered() {}

template<class Base, class Clamp, class Filter>
void Filtered<Base, Clamp, Filter>::add_sample(const sampler::Camera_sample& sample, const math::float4& color,
											   const math::Recti& tile, const math::Recti& bounds) {
	math::float4 clamped_color = clamp_.clamp(color);

	int32_t x = sample.pixel.x;
	int32_t y = sample.pixel.y;

	math::float2 relative_offset = sample.pixel_uv - math::float2(0.5f, 0.5f);

	math::float2 o = relative_offset;
	o.x += 1.f;
	o.y += 1.f;
	weight_and_add_pixel(x - 1, y - 1, o, clamped_color, tile, bounds);

	o = relative_offset;
	o.y += 1.f;
	weight_and_add_pixel(x, y - 1, o, clamped_color, tile, bounds);

	o = relative_offset;
	o.x -= 1.f;
	o.y += 1.f;
	weight_and_add_pixel(x + 1, y - 1, o, clamped_color, tile, bounds);

	o = relative_offset;
	o.x += 1.f;
	weight_and_add_pixel(x - 1, y, o, clamped_color, tile, bounds);

	// center
	weight_and_add_pixel(x, y, relative_offset, clamped_color, tile, bounds);

	o = relative_offset;
	o.x -= 1.f;
	weight_and_add_pixel(x + 1, y, o, clamped_color, tile, bounds);

	o = relative_offset;
	o.x += 1.f;
	o.y -= 1.f;
	weight_and_add_pixel(x - 1, y + 1, o, clamped_color, tile, bounds);

	o = relative_offset;
	o.y -= 1.f;
	weight_and_add_pixel(x, y + 1, o, clamped_color, tile, bounds);

	o = relative_offset;
	o.x -= 1.f;
	o.y -= 1.f;
	weight_and_add_pixel(x + 1, y + 1, o, clamped_color, tile, bounds);
}

template<class Base, class Clamp, class Filter>
void Filtered<Base, Clamp, Filter>::weight_and_add_pixel(int32_t x, int32_t y, math::float2 relative_offset,
														 const math::float4& color,
														 const math::Recti& tile, const math::Recti& bounds) {
	if (x < bounds.start.x || x >= bounds.end.x || y < bounds.start.y || y >= bounds.end.y) {
		return;
	}

	float weight = filter_->evaluate(relative_offset);

	auto d = Base::dimensions();
	if ((x < d.x - 1 && x >= tile.end.x - 1)
	||  (y < d.y - 1 && y >= tile.end.y - 1)
	||  (x <= tile.start.x && 0 != x)
	||  (y <= tile.start.y && 0 != y)) {
		Base::add_pixel_atomic(x, y, color, weight);
	} else {
		Base::add_pixel(x, y, color, weight);
	}
}

}}
