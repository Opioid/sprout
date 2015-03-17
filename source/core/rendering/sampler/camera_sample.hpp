#pragma once

#include "base/math/vector.hpp"

namespace rendering { namespace sampler {

struct Camera_sample {
	math::float2 coordinates;
	math::float2 r;
};

}}
