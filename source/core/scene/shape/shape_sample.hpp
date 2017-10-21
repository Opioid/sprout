#pragma once

#include "base/math/vector3.hpp"

namespace scene::shape {

struct Sample {
	float3 wi;
	float2 uv;
	float t;
	float pdf;
};

}
