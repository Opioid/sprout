#ifndef SU_CORE_SCENE_RAY_INL
#define SU_CORE_SCENE_RAY_INL

#include "base/math/ray.inl"
#include "scene_ray.hpp"

namespace scene {

inline Ray::Ray(float3 const& origin, float3 const& direction, float min_t, float max_t,
                uint32_t depth, uint64_t time, float wavelength /*, Properties properties*/)
    : ray(origin, direction, min_t, max_t, depth),
      time(time),
      wavelength(wavelength) /*, properties(properties)*/ {}
}  // namespace scene

#endif
