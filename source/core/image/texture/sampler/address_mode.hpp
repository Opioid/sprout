#pragma once

#include "base/math/math.hpp"
#include "base/math/vector.inl"

namespace image { namespace texture { namespace sampler {

struct Address_mode_clamp {
	static float2 f(float2 uv) {
		return math::saturate(uv);
	}

	static float3 f(float3_p uvw) {
		return math::saturate(uvw);
	}

	static int32_t increment(int32_t v, int32_t max) {
		if (v >= max) {
			return max;
		}

		return v + 1;
	}

	static int32_t lower_bound(int32_t v, int32_t /*max*/) {
		if (v < 0) {
			return 0;
		}

		return v;
	}
};

struct Address_mode_repeat {
	static float2 f(float2 uv) {
		return float2(math::frac(uv.x), math::frac(uv.y));
	}

	static float3 f(float3_p uvw) {
		return float3(math::frac(uvw.x), math::frac(uvw.y), math::frac(uvw.z));
	}

	static int32_t increment(int32_t v, int32_t max) {
		if (v >= max) {
			return 0;
		}

		return v + 1;
	}

	static int32_t lower_bound(int32_t v, int32_t max) {
		if (v < 0) {
			return max;
		}

		return v;
	}
};

}}}
