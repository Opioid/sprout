#pragma once

#include "base/math/vector.hpp"

namespace sampler {

struct Camera_sample {
	math::uint2  pixel;
	math::float2 coordinates;
	math::float2 relative_offset;
	math::float2 lens_uv;
	float        time;
};

}
