#pragma once

#include "base/simd/simd.hpp"
#include <cstdint>
//#include "base/math/vector3.hpp"

namespace scene { namespace shape { namespace triangle {

struct Intersection {
	Vector u;
	Vector v;

//	float2 uv;
	uint32_t index;
};

}}}

