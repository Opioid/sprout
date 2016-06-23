#pragma once

#include "base/math/math.hpp"
#include "base/math/vector.inl"

namespace image { namespace texture { namespace sampler {

struct Address_mode_repeat {
	float2 f(float2 uv) const {
		return float2(math::frac(uv.x), math::frac(uv.y));
	}

	int32_t increment(int32_t v, int32_t max) const {
		if (v >= max) {
			return 0;
		}

		return v + 1;
	}

	int32_t lower_bound(int32_t v, int32_t max) const {
		if (v < 0) {
			return max;
		}

		return v;
	}
};

}}}
