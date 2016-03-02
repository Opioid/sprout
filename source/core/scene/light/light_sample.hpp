#pragma once

#include "scene/shape/shape_sample.hpp"
#include "base/math/vector.hpp"

namespace scene { namespace light {

struct Sample {
	shape::Sample shape;
	math::float3  energy;
};

}}
