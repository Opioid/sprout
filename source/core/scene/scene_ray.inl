#pragma once

#include "scene_ray.hpp"
#include "base/math/ray.inl"

namespace scene {

inline Ray::Ray() {}

inline Ray::Ray(const math::vec3& origin, const math::vec3& direction,
				float min_t, float max_t,
				float time, uint32_t depth, float ior) :
	math::Oray(origin, direction, min_t, max_t),
	time(time), depth(depth), ior(ior) {}

}
