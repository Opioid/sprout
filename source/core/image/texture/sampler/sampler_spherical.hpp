#pragma once

#include "base/math/vector.hpp"

namespace image { namespace texture {

class Texture_2D;

namespace sampler {

class Spherical {
public:

	virtual math::vec3 sample_3(const Texture_2D& texture, const math::vec3& xyz) const = 0;
};

}}}
