#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace shape { namespace triangle {

struct Coordinates {
	float t;
	math::float2 uv;
};

struct Intersection {
	Coordinates c;
	uint32_t index;
};

}}}

