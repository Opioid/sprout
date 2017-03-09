#pragma once

#include "scene_ray.hpp"

namespace scene {

inline Ray::Ray(float3_p origin, float3_p direction,
				float min_t, float max_t,
				float time, uint32_t depth, float ior) :
	math::Ray(origin, direction, min_t, max_t),
	time(time), depth(depth), ior(ior) {}

}
