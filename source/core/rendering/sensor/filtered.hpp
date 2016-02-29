#pragma once

#include "base/math/rectangle.hpp"
#include "base/math/vector.hpp"

namespace sampler { struct Camera_sample; }

namespace rendering { namespace sensor {

namespace tonemapping { class Tonemapper; }

namespace filter { class Filter; }

template<class Base, class Clamp>
class Filtered : public Base {
public:

	Filtered(math::int2 dimensions,
			 const tonemapping::Tonemapper* tonemapper,
			 const Clamp& clamp,
			 const filter::Filter* filter);

	~Filtered();

	virtual int32_t filter_radius_int() const final override;

	virtual void add_sample(const sampler::Camera_sample& sample, const math::float4& color,
							const math::Recti& tile, const math::Recti& view_bounds) final override;

private:

	void weight_and_add_pixel(math::int2 pixel, math::float2 relative_offset,
							  const math::float4& color, const math::Recti& view_tile, const math::Recti& view_bounds);

	Clamp clamp_;

	const filter::Filter* filter_;
};

}}
