#pragma once

#include "base/math/vector.hpp"

namespace image { namespace texture {

class Texture_2D;

namespace sampler {

class Sampler_2D {

public:

	virtual ~Sampler_2D() {}

	virtual float  sample_1(const Texture_2D& texture, float2 uv) const = 0;
	virtual float2 sample_2(const Texture_2D& texture, float2 uv) const = 0;
	virtual float3 sample_3(const Texture_2D& texture, float2 uv) const = 0;

	virtual float  sample_1(const Texture_2D& texture, float2 uv, int32_t element) const = 0;
	virtual float2 sample_2(const Texture_2D& texture, float2 uv, int32_t element) const = 0;
	virtual float3 sample_3(const Texture_2D& texture, float2 uv, int32_t element) const = 0;

	virtual float2 address(float2 uv) const = 0;
};

}}}
