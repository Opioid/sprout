#pragma once

#include "base/math/vector4.hpp"

namespace sampler { struct Camera_sample; }

namespace rendering { namespace sensor {

template<class Base, class Clamp>
class Unfiltered : public Base {

public:

	Unfiltered(int2 dimensions, float exposure, const Clamp& clamp);

	virtual int32_t filter_radius_int() const override final;

	virtual int4 isolated_tile(const int4& tile) const override final;

	virtual void add_sample(const sampler::Camera_sample& sample, const float4& color,
							const int4& isolated_bounds, const int4& bounds) override final;

private:

	Clamp clamp_;
};

}}
