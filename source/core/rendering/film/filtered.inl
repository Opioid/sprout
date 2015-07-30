#include "filtered.hpp"
#include "sampler/camera_sample.hpp"

namespace rendering { namespace film {

template<typename Filter>
Filtered<Filter>::Filtered(const math::uint2& dimensions, float exposure,
						   tonemapping::Tonemapper* tonemapper, std::unique_ptr<Filter> filter) :
	Film(dimensions, exposure, tonemapper), filter_(std::move(filter)) {}

template<typename Filter>
Filtered<Filter>::~Filtered() {}

template<typename Filter>
void Filtered<Filter>::add_sample(const sampler::Camera_sample& sample, const math::float3& color, const Rectui& tile) {
	uint32_t x = static_cast<uint32_t>(sample.coordinates.x);
	uint32_t y = static_cast<uint32_t>(sample.coordinates.y);

	math::float2 o = sample.relative_offset;
	o.x += 1.f;
	o.y += 1.f;
	add_pixel(x - 1, y - 1, o, color, tile);

	o = sample.relative_offset;
	o.y += 1.f;
	add_pixel(x, y - 1, o, color, tile);

	o = sample.relative_offset;
	o.x -= 1.f;
	o.y += 1.f;
	add_pixel(x + 1, y - 1, o, color, tile);

	o = sample.relative_offset;
	o.x += 1.f;
	add_pixel(x - 1, y, o, color, tile);

	// center
	add_pixel(x, y, sample.relative_offset, color, tile);

	o = sample.relative_offset;
	o.x -= 1.f;
	add_pixel(x + 1, y, o, color, tile);

	o = sample.relative_offset;
	o.x += 1.f;
	o.y -= 1.f;
	add_pixel(x - 1, y + 1, o, color, tile);

	o = sample.relative_offset;
	o.y -= 1.f;
	add_pixel(x, y + 1, o, color, tile);

	o = sample.relative_offset;
	o.x -= 1.f;
	o.y -= 1.f;
	add_pixel(x + 1, y + 1, o, color, tile);
}

template<typename Filter>
void Filtered<Filter>::add_pixel(uint32_t x, uint32_t y, math::float2 relative_offset,
								 const math::float3& color, const Rectui& tile) {
	float weight = filter_->evaluate(relative_offset);

	if ((tile.start.x == x && 0 != x)
	||  (tile.end.x - 1 == x && x < dimensions().x - 1)
	||  (tile.start.y == y && 0 != y)
	||  (tile.end.y - 1 == y && y < dimensions().y - 1)) {
		add_pixel_atomic(x, y, color, weight);
	} else {
		Film::add_pixel(x, y, color, weight);
	}
}

}}
