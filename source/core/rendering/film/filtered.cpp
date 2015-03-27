#include "filtered.hpp"
#include "filter/filter.hpp"
#include "sampler/camera_sample.hpp"

namespace rendering { namespace film {

Filtered::Filtered(const math::uint2& dimensions, float exposure, tonemapping::Tonemapper* tonemapper, filter::Filter* filter) :
	Film(dimensions, exposure, tonemapper), filter_(filter) {}

Filtered::~Filtered() {
	delete filter_;
}

void Filtered::add_sample(const sampler::Camera_sample& sample, const math::float3& color, const Rectui& tile) {
	uint32_t x = static_cast<uint32_t>(sample.coordinates.x);
	uint32_t y = static_cast<uint32_t>(sample.coordinates.y);

	bool left_edge   = false;
	bool right_edge  = false;
	bool top_edge    = false;
	bool bottom_edge = false;

	if (tile.start.x == x && 0 != x) {
		left_edge = true;
	} else if (tile.end.x - 1 == x && x < dimensions().x - 1) {
		right_edge = true;
	}

	if (tile.start.y == y && 0 != y) {
		top_edge = true;
	} else if (tile.end.y - 1 == y && y < dimensions().x - 1) {
		bottom_edge = true;
	}

	math::float2 o = sample.relative_offset;
	o.x += 1.f;
	o.y += 1.f;
	float w = filter_->evaluate(o);

	if (left_edge || top_edge) {
		add_pixel_atomic(x - 1, y - 1, color, w);
	} else {
		add_pixel(x - 1, y - 1, color, w);
	}

	o = sample.relative_offset;
	o.y += 1.f;
	w = filter_->evaluate(o);

	if (top_edge) {
		add_pixel_atomic(x, y - 1, color, w);
	} else {
		add_pixel(x, y - 1, color, w);
	}

	o = sample.relative_offset;
	o.x -= 1.f;
	o.y += 1.f;
	w = filter_->evaluate(o);

	if (right_edge || top_edge) {
		add_pixel_atomic(x + 1, y - 1, color, w);
	} else {
		add_pixel(x + 1, y - 1, color, w);
	}

	o = sample.relative_offset;
	o.x += 1.f;
	w = filter_->evaluate(o);

	if (left_edge) {
		add_pixel_atomic(x - 1, y, color, w);
	} else {
		add_pixel(x - 1, y, color, w);
	}

	// center
	w = filter_->evaluate(sample.relative_offset);

	if (left_edge || right_edge || top_edge || bottom_edge) {
		add_pixel_atomic(x, y, color, w);
	} else {
		add_pixel(x, y, color, w);
	}

	o = sample.relative_offset;
	o.x -= 1.f;
	w = filter_->evaluate(o);

	if (right_edge) {
		add_pixel_atomic(x + 1, y, color, w);
	} else {
		add_pixel(x + 1, y, color, w);
	}

	o = sample.relative_offset;
	o.x += 1.f;
	o.y -= 1.f;
	w = filter_->evaluate(o);

	if (left_edge || bottom_edge) {
		add_pixel_atomic(x - 1, y + 1, color, w);
	} else {
		add_pixel(x - 1, y + 1, color, w);
	}

	o = sample.relative_offset;
	o.y -= 1.f;
	w = filter_->evaluate(o);

	if (bottom_edge) {
		add_pixel_atomic(x, y + 1, color, w);
	} else {
		add_pixel(x, y + 1, color, w);
	}

	o = sample.relative_offset;
	o.x -= 1.f;
	o.y -= 1.f;
	w = filter_->evaluate(o);

	if (right_edge || bottom_edge) {
		add_pixel_atomic(x + 1, y + 1, color, w);
	} else {
		add_pixel(x + 1, y + 1, color, w);
	}
}

}}
