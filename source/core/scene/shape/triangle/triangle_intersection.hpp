#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace shape { namespace triangle {

struct Intersection {
	math::float2 uv;
	uint32_t index;
};

}}}

