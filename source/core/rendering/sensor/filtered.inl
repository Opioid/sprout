#ifndef SU_CORE_RENDERING_SENSOR_FILTERED_INL
#define SU_CORE_RENDERING_SENSOR_FILTERED_INL

#include "filtered.hpp"
#include "filter/sensor_filter.hpp"
#include "sampler/camera_sample.hpp"
#include "base/math/vector4.inl"

namespace rendering::sensor {

template<class Base, class Clamp>
Filtered<Base, Clamp>::Filtered(int2 dimensions, float exposure, const Clamp& clamp,
								filter::Filter const* filter) :
	Base(dimensions, exposure), clamp_(clamp), filter_(filter) {}

template<class Base, class Clamp>
Filtered<Base, Clamp>::Filtered(int2 dimensions, float exposure, const Texture_ptr& backplate,
								const Clamp& clamp, filter::Filter const* filter) :
	Base(dimensions, exposure, backplate), clamp_(clamp), filter_(filter) {}

template<class Base, class Clamp>
Filtered<Base, Clamp>::~Filtered() {
	delete filter_;
}

template<class Base, class Clamp>
int32_t Filtered<Base, Clamp>::filter_radius_int() const {
	return static_cast<int32_t>(std::ceil(filter_->radius()));
}

template<class Base, class Clamp>
int4 Filtered<Base, Clamp>::isolated_tile(int4 const& tile) const {
	int32_t const r = filter_radius_int();
	return tile + int4(r, r, -r, -r);
}

template<class Base, class Clamp>
void Filtered<Base, Clamp>::add_sample(sampler::Camera_sample const& sample, float4 const& color,
									   int4 const& isolated_bounds, int4 const& bounds) {
	const float4 clamped_color = clamp_.clamp(color);

	int32_t const x = bounds[0] + sample.pixel[0];
	int32_t const y = bounds[1] + sample.pixel[1];

	float const ox = sample.pixel_uv[0] - 0.5f;
	float const oy = sample.pixel_uv[1] - 0.5f;

	float const wx0 = filter_->evaluate(ox + 1.f);
	float const wx1 = filter_->evaluate(ox);
	float const wx2 = filter_->evaluate(ox - 1.f);

	float const wy0 = filter_->evaluate(oy + 1.f);
	float const wy1 = filter_->evaluate(oy);
	float const wy2 = filter_->evaluate(oy - 1.f);

	// 1. row
	add_weighted_pixel(int2(x - 1, y - 1), wx0 * wy0, clamped_color, isolated_bounds, bounds);
	add_weighted_pixel(int2(x, y - 1),	   wx1 * wy0, clamped_color, isolated_bounds, bounds);
	add_weighted_pixel(int2(x + 1, y - 1), wx2 * wy0, clamped_color, isolated_bounds, bounds);

	// 2. row
	add_weighted_pixel(int2(x - 1, y), wx0 * wy1, clamped_color, isolated_bounds, bounds);
	add_weighted_pixel(int2(x, y),	   wx1 * wy1, clamped_color, isolated_bounds, bounds);
	add_weighted_pixel(int2(x + 1, y), wx2 * wy1, clamped_color, isolated_bounds, bounds);

	// 3. row
	add_weighted_pixel(int2(x - 1, y + 1), wx0 * wy2, clamped_color, isolated_bounds, bounds);
	add_weighted_pixel(int2(x, y + 1),	   wx1 * wy2, clamped_color, isolated_bounds, bounds);
	add_weighted_pixel(int2(x + 1, y + 1), wx2 * wy2, clamped_color, isolated_bounds, bounds);

	/*
	weight_and_add_pixel(int2(x - 1, y - 1), float2(ox + 1.f, oy + 1.f),
						 clamped_color, isolated_bounds, bounds);

	weight_and_add_pixel(int2(x, y - 1), float2(ox, oy + 1.f),
						 clamped_color, isolated_bounds, bounds);

	weight_and_add_pixel(int2(x + 1, y - 1), float2(ox - 1.f, oy + 1.f),
						 clamped_color, isolated_bounds, bounds);

	weight_and_add_pixel(int2(x - 1, y), float2(ox + 1.f, oy),
						 clamped_color, isolated_bounds, bounds);

	// center
	weight_and_add_pixel(int2(x, y), float2(ox, oy),
						 clamped_color, isolated_bounds, bounds);

	weight_and_add_pixel(int2(x + 1, y), float2(ox - 1.f, oy),
						 clamped_color, isolated_bounds, bounds);

	weight_and_add_pixel(int2(x - 1, y + 1), float2(ox + 1.f, oy - 1.f),
						 clamped_color, isolated_bounds, bounds);

	weight_and_add_pixel(int2(x, y + 1), float2(ox, oy - 1.f),
						 clamped_color, isolated_bounds, bounds);

	weight_and_add_pixel(int2(x + 1, y + 1), float2(ox - 1.f, oy - 1.f),
						 clamped_color, isolated_bounds, bounds);
*/
}

template<class Base, class Clamp>
void Filtered<Base, Clamp>::add_weighted_pixel(int2 pixel, float weight, float4 const& color,
											   int4 const& isolated_bounds, int4 const& bounds) {
//	if (pixel[0] >= bounds[0] && pixel[0] <= bounds[2]
//	&&	pixel[1] >= bounds[1] && pixel[1] <= bounds[3]) {

	// This code assumes that (isolated_)bounds contains [x_lo, y_lo, x_hi - x_lo, y_hi - y_lo]

	if (static_cast<uint32_t>(pixel[0] - bounds[0]) <= static_cast<uint32_t>(bounds[2])
	&&	static_cast<uint32_t>(pixel[1] - bounds[1]) <= static_cast<uint32_t>(bounds[3])) {
		if (static_cast<uint32_t>(pixel[0] - isolated_bounds[0])
		<=  static_cast<uint32_t>(isolated_bounds[2])
		&&	static_cast<uint32_t>(pixel[1] - isolated_bounds[1])
		<=  static_cast<uint32_t>(isolated_bounds[3])) {
			Base::add_pixel(pixel, color, weight);
		} else {
			Base::add_pixel_atomic(pixel, color, weight);
		}
	}
}

template<class Base, class Clamp>
void Filtered<Base, Clamp>::weight_and_add_pixel(int2 pixel, float2 relative_offset,
												 float4 const& color, int4 const& isolated_bounds,
												 int4 const& bounds) {
	// This code assumes that (isolated_)bounds contains [x_lo, y_lo, x_hi - x_lo, y_hi - y_lo]

	if (static_cast<uint32_t>(pixel[0] - bounds[0]) <= static_cast<uint32_t>(bounds[2])
	&&	static_cast<uint32_t>(pixel[1] - bounds[1]) <= static_cast<uint32_t>(bounds[3])) {
		float const weight = filter_->evaluate(relative_offset);

		if (static_cast<uint32_t>(pixel[0] - isolated_bounds[0])
		<=  static_cast<uint32_t>(isolated_bounds[2])
		&&	static_cast<uint32_t>(pixel[1] - isolated_bounds[1])
		<=  static_cast<uint32_t>(isolated_bounds[3])) {
			Base::add_pixel(pixel, color, weight);
		} else {
			Base::add_pixel_atomic(pixel, color, weight);
		}
	}
}

}

#endif
