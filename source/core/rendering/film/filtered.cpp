#include "filtered.hpp"
#include "filter/filter.hpp"
#include "rendering/sampler/camera_sample.hpp"

namespace rendering { namespace film {

Filtered::Filtered(const math::uint2& dimensions, float exposure, tonemapping::Tonemapper* tonemapper, filter::Filter* filter) :
	Film(dimensions, exposure, tonemapper), filter_(filter) {}

Filtered::~Filtered() {
	delete filter_;
}

void Filtered::add_sample(const sampler::Camera_sample& sample, const math::float3& color) {
	uint32_t x = static_cast<uint32_t>(sample.coordinates.x);
	uint32_t y = static_cast<uint32_t>(sample.coordinates.y);

	bool left_edge = false;
	bool right_edge = false;
	bool top_edge = false;
	bool bottom_edge = false;

	/*
	if x == start.X && x != 0 {
		leftEdge = true
	}
	if x == end.X - 1 && x < f.dimensions.X - 1 {
		rightEdge = true
	}
	if y == start.Y && y != 0 {
		topEdge = true
	}
	if y == end.Y - 1  && y < f.dimensions.Y - 1 {
		bottomEdge = true
	}
	*/

	math::float2 o = sample.relative_offset;
	o.x += 1.f;
	o.y += 1.f;
	float w = filter_->evaluate(o);

	if (left_edge || top_edge) {
	//	f.atomicAddPixel(x - 1, y - 1, color, w)
	} else {
		add_pixel(x - 1, y - 1, color, w);
	}

	o = sample.relative_offset;
	o.y += 1.f;
	w = filter_->evaluate(o);

	if (top_edge) {
	//	f.atomicAddPixel(x, y - 1, color, w)
	} else {
		add_pixel(x, y - 1, color, w);
	}

	o = sample.relative_offset;
	o.x -= 1.f;
	o.y += 1.f;
	w = filter_->evaluate(o);

	if (right_edge || top_edge) {
	//	f.atomicAddPixel(x + 1, y - 1, color, w)
	} else {
		add_pixel(x + 1, y - 1, color, w);
	}

	o = sample.relative_offset;
	o.x += 1.f;
	w = filter_->evaluate(o);

	if (left_edge) {
	//	f.atomicAddPixel(x - 1, y, color, w)
	} else {
		add_pixel(x - 1, y, color, w);
	}

	// center
	w = filter_->evaluate(sample.relative_offset);

	if (left_edge || right_edge || top_edge || bottom_edge) {
	//	f.atomicAddPixel(x, y, color, w)
	} else {
		add_pixel(x, y, color, w);
	}

	o = sample.relative_offset;
	o.x -= 1.f;
	w = filter_->evaluate(o);

	if (right_edge) {
	//	f.atomicAddPixel(x + 1, y, color, w)
	} else {
		add_pixel(x + 1, y, color, w);
	}

	o = sample.relative_offset;
	o.x += 1.f;
	o.y -= 1.f;
	w = filter_->evaluate(o);

	if (left_edge || bottom_edge) {
	//	f.atomicAddPixel(x - 1, y + 1, color, w)
	} else {
		add_pixel(x - 1, y + 1, color, w);
	}

	o = sample.relative_offset;
	o.y -= 1.f;
	w = filter_->evaluate(o);

	if (bottom_edge) {
	//	f.atomicAddPixel(x, y + 1, color, w)
	} else {
		add_pixel(x, y + 1, color, w);
	}

	o = sample.relative_offset;
	o.x -= 1.f;
	o.y -= 1.f;
	w = filter_->evaluate(o);

	if (right_edge || bottom_edge) {
	//	f.atomicAddPixel(x + 1, y + 1, color, w)
	} else {
		add_pixel(x + 1, y + 1, color, w);
	}
}

}}
