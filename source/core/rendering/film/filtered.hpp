#pragma once

#include "film.hpp"

namespace rendering { namespace film {

template<typename Filter>
class Filtered : public Film {
public:

	Filtered(const math::uint2& dimensions, float exposure, tonemapping::Tonemapper* tonemapper, const Filter& filter);
	~Filtered();

	virtual void add_sample(const sampler::Camera_sample& sample, const math::float3& color, const Rectui& tile) final override;

private:

	void add_pixel(uint32_t x, uint32_t y, math::float2 relative_offset, const math::float3& color, const Rectui& tile);

	Filter filter_;
};

}}
