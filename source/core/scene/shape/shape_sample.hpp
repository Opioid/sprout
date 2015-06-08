#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace shape {

struct Sample {
	math::float3 wi;
	math::float2 uv;
	float t;
	float pdf;
};

}}
