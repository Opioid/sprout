#pragma once

#include "base/math/math.hpp"
#include "base/math/vector.hpp"

namespace image { namespace texture { namespace sampler {

struct Address_mode_repeat {
	math::float2 f(math::float2 uv) const {
		return math::float2(math::frac(uv.x), math::frac(uv.y));
	}

	int32_t increment(int32_t v, int32_t limit) const {
		if (v >= limit) {
			return 0;
		}

		return v + 1;
	}

	int32_t lower_bound(int32_t v, int32_t limit) const {
		if (v < 0) {
			return limit - 1;
		}

		return v;
	}
};

}}}
