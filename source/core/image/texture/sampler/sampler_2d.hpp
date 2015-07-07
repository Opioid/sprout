#pragma once

#include "base/math/vector.hpp"

namespace image {

class Texture_2D;

namespace sampler {

class Sampler_2D {
public:

	virtual float        sample1(const Texture_2D& texture, math::float2 uv) const = 0;
	virtual math::float2 sample2(const Texture_2D& texture, math::float2 uv) const = 0;
	virtual math::float3 sample3(const Texture_2D& texture, math::float2 uv) const = 0;

	virtual math::float2 address(math::float2 uv) const = 0;
};

}}
