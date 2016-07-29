#pragma once

#include "base/math/ray.hpp"
#include "base/math/vector.hpp"

namespace scene {

struct Ray : public math::Oray {
	Ray();
	Ray(float3_p origin, float3_p direction,
		float min_t = 0.f, float max_t = 1.f,
		float time = 0.f, uint32_t depth = 0, float ior = 1.f);

	float	 time;
	uint32_t depth;
	float    ior;
};

}
