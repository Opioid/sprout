#pragma once

#include "opaque.hpp"

namespace rendering { namespace film {

class Unfiltered : public Opaque {
public:

	Unfiltered(const math::uint2& dimensions, float exposure, tonemapping::Tonemapper* tonemapper);

	virtual void add_sample(const sampler::Camera_sample& sample, const math::float4& color, const Rectui& tile);
};

}}
