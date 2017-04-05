#pragma once

#include "base/math/vector3.hpp"

namespace image { namespace texture {

class Texture;

namespace sampler {

class Sampler_3D {

public:

	virtual ~Sampler_3D() {}

	virtual float  sample_1(const Texture& texture, const float3& uvw) const = 0;
	virtual float2 sample_2(const Texture& texture, const float3& uvw) const = 0;
	virtual float3 sample_3(const Texture& texture, const float3& uvw) const = 0;

	virtual float3 address(const float3& uvw) const = 0;
};

}}}
