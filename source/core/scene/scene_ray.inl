#ifndef SU_CORE_SCENE_RAY_INL
#define SU_CORE_SCENE_RAY_INL

#include "scene_ray.hpp"
#include "base/math/ray.inl"

namespace scene {

inline Ray::Ray(const float3& origin, const float3& direction, float min_t, float max_t,
				uint32_t depth, float time, float wavelength, float ior, Properties properties) :
	math::Ray(origin, direction, min_t, max_t, depth),
	time(time), wavelength(wavelength), ior(ior), properties(properties) {}
}

inline bool scene::Ray::is_primary() const {
	return properties.test(Property::Primary);
}

inline void scene::Ray::set_primary(bool primary) {
	properties.set(Property::Primary, primary);
}

#endif
