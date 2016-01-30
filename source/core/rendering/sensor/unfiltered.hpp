#pragma once

#include "base/math/vector.hpp"

namespace rendering { namespace sensor {

namespace tonemapping { class Tonemapper; }

template<class Base, class Clamp>
class Unfiltered : public Base {
public:

	Unfiltered(math::int2 dimensions, float exposure,
			   const tonemapping::Tonemapper* tonemapper, const Clamp& clamp);

	virtual int32_t filter_radius_int() const final override;

	virtual void add_sample(const sampler::Camera_sample& sample, const math::float4& color,
							const math::Recti& tile, const math::Recti& view_bounds) final override;

private:

	Clamp clamp_;
};

}}
