#pragma once

#include "base/math/vector.hpp"

namespace sampler {

struct Camera_sample {
	math::int2   pixel;
	math::float2 pixel_uv;
	math::float2 lens_uv;
	float        time;
};

}
