#pragma once

#include "base/math/rectangle.hpp"
#include "base/math/vector.hpp"

namespace sampler { struct Camera_sample; }

namespace rendering { namespace sensor {

namespace filter { class Filter; }

template<class Base, class Clamp>
class Filtered : public Base {

public:

	Filtered(int2 dimensions, float exposure, const Clamp& clamp, const filter::Filter* filter);
	~Filtered();

	virtual int32_t filter_radius_int() const final override;

	virtual void add_sample(const sampler::Camera_sample& sample, float4_p,
							const math::Recti& tile, const math::Recti& bounds) final override;

private:

	void add_pixel(int2 pixel, float weight, float4_p color,
				   const math::Recti& tile, const math::Recti& bounds);

	void weight_and_add_pixel(int2 pixel, float2 relative_offset, float4_p color,
							  const math::Recti& tile, const math::Recti& bounds);

	void weight_and_add_pixel2(int2 pixel, float2 relative_offset, float4_p color,
							   const math::Recti& tile, const math::Recti& bounds);

	Clamp clamp_;

	const filter::Filter* filter_;
};

}}
