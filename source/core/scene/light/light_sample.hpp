#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace light {

struct Sample {
	math::float3 l;
	math::float3 energy;
	float t;
	float pdf;
};

}}
