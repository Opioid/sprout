#pragma once

#include "base/math/vector.hpp"

namespace image { namespace texture {

class Texture;

namespace sampler {

class Spherical {
public:

	virtual float3 sample_3(const Texture& texture, const float3& xyz) const = 0;
};

}}}
