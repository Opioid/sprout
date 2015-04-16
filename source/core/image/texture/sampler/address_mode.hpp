#pragma once

#include "base/math/vector.hpp"
#include <cmath>

namespace image { namespace sampler {

struct Address_mode_repeat {
	math::float2 f(math::float2 uv) const {
		return math::float2(uv.x - std::floor(uv.x), uv.y - std::floor(uv.y));
	}
};

}}
