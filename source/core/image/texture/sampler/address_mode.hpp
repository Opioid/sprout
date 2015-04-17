#pragma once

#include "base/math/math.hpp"
#include "base/math/vector.hpp"

namespace image { namespace sampler {

struct Address_mode_repeat {
	math::float2 f(math::float2 uv) const {
		return math::float2(math::frac(uv.x), math::frac(uv.y));
	}
};

}}
