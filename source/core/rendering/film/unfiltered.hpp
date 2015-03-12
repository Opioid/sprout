#pragma once

#include "film.hpp"

namespace film {

class Unfiltered : public Film {
public:

	Unfiltered(const math::uint2& dimensions);

	virtual void add_sample(const sampler::Camera_sample& sample, const math::float3& color);
};

}
