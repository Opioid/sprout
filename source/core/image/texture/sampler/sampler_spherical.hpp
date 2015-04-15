#pragma once

#include "base/math/vector.hpp"

namespace image {

class Texture2D;

namespace sampler {

class Spherical {
public:

	virtual math::float3 sample3(const Texture2D& texture, const math::float3& xyz) const = 0;
};

}}
