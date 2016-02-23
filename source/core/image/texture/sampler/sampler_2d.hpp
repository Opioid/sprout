#pragma once

#include "base/math/vector.hpp"

namespace image { namespace texture {

class Texture_2D;

namespace sampler {

class Sampler_2D {
public:

	virtual float        sample_1(const Texture_2D& texture, math::float2 uv) const = 0;
	virtual math::float2 sample_2(const Texture_2D& texture, math::float2 uv) const = 0;
	virtual math::vec3 sample_3(const Texture_2D& texture, math::float2 uv) const = 0;

	virtual float        sample_1(const Texture_2D& texture, math::float2 uv, int32_t element) const = 0;
	virtual math::float2 sample_2(const Texture_2D& texture, math::float2 uv, int32_t element) const = 0;
	virtual math::vec3 sample_3(const Texture_2D& texture, math::float2 uv, int32_t element) const = 0;

	virtual math::float2 address(math::float2 uv) const = 0;
};

}}}
