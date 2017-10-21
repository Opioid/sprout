#pragma once

#include "scene/shape/shape_sample.hpp"
#include "base/math/vector3.hpp"

namespace scene::light {

struct Sample {
	shape::Sample shape;
	float3		  radiance;
};

}
