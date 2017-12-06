#ifndef SU_CORE_SCENE_RAY_INL
#define SU_CORE_SCENE_RAY_INL

#include "scene_ray.hpp"
#include "base/math/ray.inl"

namespace scene {

inline Ray::Ray(const float3& origin, const float3& direction, float min_t, float max_t,
				float time, uint32_t depth, flags::Flags<Properties> properties, float ior) :
				math::Ray(origin, direction, min_t, max_t),
				time(time), depth(depth), properties(properties), ior(ior) {}
}

#endif
