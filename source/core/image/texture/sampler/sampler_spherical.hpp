#pragma once

#include "base/math/vector.hpp"

namespace image { namespace texture {

class Texture_2D;

namespace sampler {

class Spherical {
public:

	virtual math::float3 sample_3(const Texture_2D& texture, const math::float3& xyz) const = 0;
};

}}}
