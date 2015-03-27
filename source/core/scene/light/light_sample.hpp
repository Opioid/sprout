#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace light {

struct Sample {
	math::float3 energy;
	math::float3 l;
	float t;
	float pdf;
};

}}
