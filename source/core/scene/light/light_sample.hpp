#pragma once

#include "scene/shape/shape_sample.hpp"
#include "base/math/vector.hpp"

namespace scene { namespace light {

struct Sample {
	shape::Sample shape;
	float3  radiance;
};

}}
