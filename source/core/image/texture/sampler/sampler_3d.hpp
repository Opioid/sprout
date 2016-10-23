#pragma once

#include "base/math/vector.hpp"

namespace image { namespace texture {

class Texture;

namespace sampler {

class Sampler_3d {

public:

	virtual ~Sampler_3d() {}

	virtual float  sample_1(const Texture& texture, float3_p uvw) const = 0;
	virtual float2 sample_2(const Texture& texture, float3_p uvw) const = 0;
	virtual float3 sample_3(const Texture& texture, float3_p uvw) const = 0;

	virtual float3 address(float3_p uvw) const = 0;
};

}}}
